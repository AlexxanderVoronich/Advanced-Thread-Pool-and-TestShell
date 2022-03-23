
#include "DllHeader.h"
#include "ThreadPoolSettings.h"
#include "AdvThreadPool.h"
#include "AdvMacros.h"
#include "QMainWindow.h"
#include <QApplication>

std::shared_ptr<cDllQtApplicationTaskContainer> cDllThreadPool::m_qt_app = nullptr;

extern "C" IMPORT_EXPORT void openPool()
{
    cDllThreadPool::startThreadPool(2, 2, 1, 1, "../etc/ThreadPoolSettings.ini");
}

cDllQtApplicationTaskContainer::cDllQtApplicationTaskContainer()
{
    
}

int cDllQtApplicationTaskContainer::longTaskFunction()
{
    int c = 0;
    QApplication a(c, 0);
    cAdvPoolGUI w;
    w.createThreadPoolShell();//create linking between thead pool and it's dialog
    w.show();
    return a.exec();
}

void cDllQtApplicationTaskContainer::stopLongTaskFunction()
{
    
}

void cDllThreadPool::startThreadPool(int _unsharedThreadsQuantity,
    int _sharedThreadsQuantity,
    int _task_mode,
    int _stretch_mode,
    std::string _file)
{
    cPoolModes modes;
    modes.m_threadPoolMode = (eThreadPoolMode)_task_mode;// eThreadPoolMode::REPEATED_TASK_MODE;
    modes.m_stretchMode = (eStretchMode)_stretch_mode;// eStretchMode::YES_STRETCH;
    cAdvThreadPool::startThreadPool(_unsharedThreadsQuantity, _sharedThreadsQuantity,
        modes, QString::fromStdString(_file));

    m_qt_app = std::make_shared<cDllQtApplicationTaskContainer>();

    auto pLongTask = new cLongTask<cDllQtApplicationTaskContainer, int>(m_qt_app.get(),
        &cDllQtApplicationTaskContainer::longTaskFunction,
        &cDllQtApplicationTaskContainer::stopLongTaskFunction,
        QString("QtApplication"));

    auto pFuture = cAdvThreadPool::launchRunnableObject<int, cLongTask<cDllQtApplicationTaskContainer, int>>(pLongTask);
    if (pFuture != nullptr)
    {
        //m_threadPoolDialog = std::make_shared<cAdvPoolGUI>(m_qt_app.get());
        //m_threadPoolDialog->createThreadPoolShell();//create linking between thead pool and it's dialog
    }

}


void cDllThreadPool::stopThreadPool()
{
    cAdvThreadPool::stopThreadPool();
}

int cDllThreadPool::createAndLaunchLongTask(std::string _longTaskName, FPTR _run_delegate, FPTR _stop_delegate)
{
    auto long_task_wrapper = std::make_shared<cDllLongTaskContainer>(_run_delegate, _stop_delegate);

    auto pLongTask = new cLongTask<cDllLongTaskContainer, int>(long_task_wrapper.get(),
        &cDllLongTaskContainer::longTaskFunction,
        &cDllLongTaskContainer::stopLongTaskFunction,
        QString::fromStdString(_longTaskName));

    auto pFuture = cAdvThreadPool::launchRunnableObject<int, cLongTask<cDllLongTaskContainer, int>>(pLongTask);
    if (pFuture != nullptr)
    {
        return 0;
    }
    return 0;

}

int cDllThreadPool::createAndLaunchRepeatTask(FPTR _run_delegate, FPTR _stop_delegate)
{
    /*auto repeatTask = macros_CreateAndLaunchRepeatTask(repeatTask, \
        pTaskContainer, \
        Tasks, \
        repeatTaskFunction, \
        qint32, \
        1000, \
        repeatTaskName);*/
    return 0;
}
