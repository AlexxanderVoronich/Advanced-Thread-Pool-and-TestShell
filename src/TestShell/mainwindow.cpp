#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "AdvThreadPool/AdvThreadPool.h"
#include "AdvThreadPool/AdvPoolGUI.h"
#include "AdvThreadPool/AdvMacros.h"
#include "AdvThreadPool/Runnable.h"

MainWindow::MainWindow(QWidget *_parent) :
    QMainWindow(_parent),
    m_ui(new Ui::AdvancedThreadPoolTestShell)
{
    m_ui->setupUi(this);
    QIntValidator v(0, 900, this);
    m_ui->lineEdit_ShortTaskQuantity->setValidator(&v);

    m_warningJournal = new cWarningJournal(this, QString("Notification box"));
    addDockWidget(Qt::BottomDockWidgetArea, m_warningJournal->m_warningDockWidget);
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_warningJournal->m_warningDockWidget->toggleViewAction());

    cPoolModes modes;
    modes.m_threadPoolMode = eThreadPoolMode::REPEATED_TASK_MODE;
    modes.m_stretchMode = eStretchMode::YES_STRETCH;

    cAdvThreadPool::startThreadPool(2, 2, modes, "../etc/ThreadPoolSettings.ini");
    //cDllThreadPool::startThreadPool(2, 2, 1, 1, "../etc/ThreadPoolSettings.ini");
    
    m_taskContainer = new Tasks();
    m_shortTaskGenerator = new ShortTaskGenerator();
    createActions();
    createConnections();

    m_toolBar = addToolBar(tr( "Object Operations" ));
    addToolBar(Qt::TopToolBarArea, m_toolBar);

    m_toolBar->addAction(m_threadPoolAction);
    m_toolBar->addAction(m_exitAction);

    m_threadPoolDialog = new cAdvPoolGUI(this);
    m_threadPoolDialog->createThreadPoolShell();//create linking between thead pool and it's dialog

}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::createActions()
{
    m_threadPoolAction = new QAction(tr( "Advanced Thread Pool"), this);
    m_threadPoolAction->setObjectName("AdvThreadPool");
    m_threadPoolAction->setVisible(true);
    m_threadPoolAction->setEnabled(true);
    m_threadPoolAction->setIcon(QIcon(":/images/IconThreadPool.png"));

    m_exitAction = new QAction(tr( "Exit"), this);
    m_exitAction->setObjectName("Exit");
    m_exitAction->setVisible(true);
    m_exitAction->setEnabled(true);
    m_exitAction->setIcon(QIcon(":/images/IconExit.png"));
}

void MainWindow::createConnections()
{
    connect(this, SIGNAL(signal_AddWarning(QString, eLogWarning)), m_warningJournal, SLOT(slot_addWarning(QString, eLogWarning)));
    connect(m_threadPoolAction, SIGNAL(triggered()), this, SLOT(slot_OnThreadPool()));
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(slot_OnExit()));
    connect(m_ui->pushButton_LongTask, SIGNAL(clicked()), this, SLOT(slot_CreateLongTask()));
    connect(m_ui->pushButton_RepeatTask, SIGNAL(clicked()), this, SLOT(slot_CreateRepeatTask()));
    connect(m_ui->pushButton_ShortTaskGenerator, SIGNAL(clicked()), this, SLOT(slot_ShortTaskGeneratorLaunch()));
    connect(m_ui->pushButton_LongTask_Stop, SIGNAL(clicked()), this, SLOT(slot_StopLongTask()));
    connect(m_ui->pushButton_RepeatTask_Stop, SIGNAL(clicked()), this, SLOT(slot_StopRepeatTask()));
    connect(m_ui->pushButton_ShortTaskGenerator_Stop, SIGNAL(clicked()), this, SLOT(slot_StopShortTaskGenerator()));
    //connect(cAdvThreadPool::getInstance().getEmitter(), SIGNAL())
}

void MainWindow::slot_OnThreadPool()
{
    m_threadPoolDialog->show();
}

void MainWindow::slot_OnExit()
{
    cAdvThreadPool::stopThreadPool();
    exit(0);
}

void MainWindow::closeEvent(QCloseEvent *_event)
{
    cAdvThreadPool::stopThreadPool();
    _event->accept();
}

void MainWindow::slot_CreateLongTask()
{
    cleanLongFuturesIfNeed();

    QString text = m_ui->lineEdit_LongTaskName->text();

    if(!text.isEmpty())
    {
        QString longTaskName = QString("Long task '%1'").arg(text);
        auto it = std::find_if(m_taskContainer->m_longTaskFutures.begin(),
                        m_taskContainer->m_longTaskFutures.end(),
                        [longTaskName](std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>> _future)
                        {
                            if(_future->m_taskDescription == longTaskName)
                                return true;
                            else
                                return false;
                        });

        if(it == m_taskContainer->m_longTaskFutures.end())//task with this name absents
        {
            //create object with task-function
            LongTaskContainer* longTaskObject = new LongTaskContainer(longTaskName);
            //std::cout<<"ptr to longTaskObject = "<<longTaskObject<<std::endl;
            //create new long task
            auto pLongTask = new cLongTask<LongTaskContainer, qint32>(longTaskObject,
                                                    &LongTaskContainer::longTaskFunction,
                                                    &LongTaskContainer::stopLongTaskFunction,
                                                    longTaskName);

            auto pFuture = cAdvThreadPool::launchRunnableObject<qint32, cLongTask<LongTaskContainer, qint32>>(pLongTask);
            if(pFuture != nullptr)
            {
                m_taskContainer->m_longTaskFutures.push_back(pFuture);
                emit signal_AddWarning(QString("Create %1").arg(longTaskName), eLogWarning::MESSAGE);
            }
            else
                emit signal_AddWarning(QString("Error of creation: no free unshared thread"), eLogWarning::WARNING);

        }
        else //task already exists
        {
            emit signal_AddWarning(QString("Error of creation: %1 already exists").arg(longTaskName), eLogWarning::WARNING);
        }
    }
}

void MainWindow::slot_StopLongTask()
{
    cleanLongFuturesIfNeed();

    QString text = m_ui->lineEdit_LongTaskName->text();

    if(!text.isEmpty())
    {
        QString longTaskName = QString("Long task '%1'").arg(text);
        auto foundFuture = std::find_if(m_taskContainer->m_longTaskFutures.begin(),
            m_taskContainer->m_longTaskFutures.end(),
            [longTaskName](std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>> _future)
            {
                if(_future->m_taskDescription == longTaskName)
                    return true;
                else
                    return false;
            });

        if(foundFuture != m_taskContainer->m_longTaskFutures.end())//task with this name was found
        {
            (*foundFuture)->m_task->stopRunnable();//stop long task
            emit signal_AddWarning(QString("Stop %1").arg(longTaskName), eLogWarning::MESSAGE);

        }
        else
        {
            emit signal_AddWarning(QString("Error of stop: %1 don't exists").arg(longTaskName), eLogWarning::WARNING);
        }
    }
}

void MainWindow::cleanLongFuturesIfNeed()
{
    std::vector<std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>>> temp_vector(m_taskContainer->m_longTaskFutures.size());

    auto it = std::copy_if(m_taskContainer->m_longTaskFutures.begin(),
                           m_taskContainer->m_longTaskFutures.end(),
                           temp_vector.begin(),
                           [](std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>> _future)
                           {
                               return _future->m_ready; //task was executed or not
                           });

    temp_vector.resize(std::distance(temp_vector.begin(),it));  // shrink container to new size

    for(auto temp_future: temp_vector)
    {
        QString longTaskName = temp_future->m_taskDescription;
        emit signal_AddWarning(QString("Delete object what incapsulate %1").arg(longTaskName), eLogWarning::WARNING);

        LongTaskContainer* ptr = static_cast<LongTaskContainer*>(temp_future->m_hostObject);

        //delete object that contains task-function
        if(ptr != nullptr)
        {
            //std::cout<<"delete pLongTaskObject = "<<ptr<<std::endl;
            delete ptr;
        }
    }

    if(!temp_vector.empty())
    {
        //remove future from array
        m_taskContainer->m_longTaskFutures.remove_if(
            [](std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>> _future)
        {
            return _future->m_ready; //task was executed or not
        });

        temp_vector.clear();
    }
}


void MainWindow::slot_CreateRepeatTask()
{
    QString text = m_ui->lineEdit_RepeatTaskName->text();

    if(!text.isEmpty())
    {
        QString repeatTaskName = QString("Repeat task '%1'").arg(text);
        auto it = find_if(m_taskContainer->m_repeatTaskFutures.begin(),
                m_taskContainer->m_repeatTaskFutures.end(),
                [repeatTaskName](std::shared_ptr<cAdvFuture<qint32, cRepeatTask<Tasks, qint32>>> _future)
                {
                  if(_future->m_taskDescription == repeatTaskName)
                      return true;
                  else
                      return false;
                });

        if(it == m_taskContainer->m_repeatTaskFutures.end())//task with this name absents
        {
            //1)create new repeat task (usual variant)
/*            auto repeatTask = new cRepeatTask<Tasks, qint32>(pTaskContainer,
                                                    &Tasks::repeatTaskFunction,
                                                    1000,
                                                    repeatTaskName);
            auto future_of_repeatTask = cAdvThreadPool::launchRunnableObject<qint32, cRepeatTask<Tasks, qint32>>(repeatTask);
*/
            //2) create new repeat task (macross variant)
             
             auto repeatTask = macros_CreateAndLaunchRepeatTask(repeatTask,\
                                                                m_taskContainer,\
                                                                Tasks,\
                                                                repeatTaskFunction,\
                                                                qint32,\
                                                                1000,\
                                                                repeatTaskName);

             if (future_of_repeatTask != nullptr)
             {
                 m_taskContainer->m_repeatTaskFutures.push_back(future_of_repeatTask);
                 emit signal_AddWarning(QString("Create %1").arg(repeatTaskName), eLogWarning::MESSAGE);
             }
             else
                 emit signal_AddWarning(QString("Error of repeat task launch"), eLogWarning::WARNING);

        }
        else //task already exists
        {
            emit signal_AddWarning(QString("Error of creation: %1 already exists").arg(repeatTaskName), eLogWarning::WARNING);
        }
    }
}

void MainWindow::slot_StopRepeatTask()
{
    QString text = m_ui->lineEdit_RepeatTaskName->text();

    if(!text.isEmpty())
    {
        QString repeatTaskName = QString("Repeat task '%1'").arg(text);
        auto foundFuture = find_if(
            m_taskContainer->m_repeatTaskFutures.begin(),
            m_taskContainer->m_repeatTaskFutures.end(),
            [repeatTaskName](std::shared_ptr<cAdvFuture<qint32, cRepeatTask<Tasks, qint32>>> _future)
            {
               if(_future->m_taskDescription == repeatTaskName)
                   return true;
               else
                   return false;
            });

        if(foundFuture == m_taskContainer->m_repeatTaskFutures.end())//task with this name was found
        {
            emit signal_AddWarning(QString("Error of stop: %1 don't exists").arg(repeatTaskName), eLogWarning::WARNING);
            return;
        }

        cAdvThreadPool::stopRepeatTask((*foundFuture)->m_repeatTaskId);

        //remove task from array
        m_taskContainer->m_repeatTaskFutures.remove_if(
        [repeatTaskName](std::shared_ptr<cAdvFuture<qint32, cRepeatTask<Tasks, qint32>>> _future)
        {
            if(_future->m_taskDescription == repeatTaskName)
                return true;
            else
                return false;
        });

        emit signal_AddWarning(QString("Stop %1").arg(repeatTaskName), eLogWarning::MESSAGE);
    }
}

void MainWindow::slot_ShortTaskGeneratorLaunch()
{
    QString text = m_ui->lineEdit_ShortTaskQuantity->text();

    if(!text.isEmpty())
    {
        qint32 shortTaskQuantity = text.toInt();
        if(m_shortTaskGenerator == nullptr)
        {
            emit signal_AddWarning(QString("Error: Generator is null"), eLogWarning::WARNING);
        }
        else if(!m_shortTaskGenerator->start(shortTaskQuantity))
        {
            emit signal_AddWarning(QString("Error: Start of Generator"), eLogWarning::WARNING);
        }
        else
        {
            emit signal_AddWarning(QString("Start of Short Task Generator"), eLogWarning::MESSAGE);
        }
    }
    else
        emit signal_AddWarning(QString("Error: Start of Generator: bad value"), eLogWarning::WARNING);

}

void MainWindow::slot_StopShortTaskGenerator()
{
    if(m_shortTaskGenerator == nullptr)
    {
        emit signal_AddWarning(QString("Error: Generator is null"), eLogWarning::WARNING);
    }
    else
    {
        m_shortTaskGenerator->stop();
        emit signal_AddWarning(QString("Stop of Short Task Generator"), eLogWarning::MESSAGE);
    }
}
