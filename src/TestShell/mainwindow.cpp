#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "AdvThreadPool/AdvThreadPool.h"
#include "AdvThreadPool/AdvPoolGUI.h"
#include "AdvThreadPool/AdvMacros.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AdvancedThreadPoolTestShell)
{
    ui->setupUi(this);
    QIntValidator v(0, 900, this);
    ui->lineEdit_ShortTaskQuantity->setValidator(&v);

    warningJournal = new cWarningJournal(this, QString("Notification box"));
    addDockWidget(Qt::BottomDockWidgetArea, warningJournal->m_warningDockWidget);
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(warningJournal->m_warningDockWidget->toggleViewAction());

    cPoolModes modes;
    modes.m_threadPoolMode = eThreadPoolMode::REPEATED_TASK_MODE;
    modes.m_stretchMode = eStretchMode::YES_STRETCH;
    cAdvThreadPool::startThreadPool(2, 2, modes, "../etc/ThreadPoolSettings.ini");

    pTaskContainer = new Tasks();
    pShortTaskGenerator = new ShortTaskGenerator();
    createActions();
    createConnections();

    pToolBar = addToolBar(tr( "Object Operations" ));
    addToolBar(Qt::TopToolBarArea, pToolBar);

    pToolBar->addAction(threadPoolAction);
    pToolBar->addAction(exitAction);

    pThreadPoolDialog = new cAdvPoolGUI(this);
    pThreadPoolDialog->createThreadPoolShell();//create linking between thead pool and it's dialog

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createActions()
{
    threadPoolAction = new QAction(tr( "Advanced Thread Pool"), this);
    threadPoolAction->setObjectName("AdvThreadPool");
    threadPoolAction->setVisible(true);
    threadPoolAction->setEnabled(true);
    threadPoolAction->setIcon(QIcon(":/images/IconThreadPool.png"));

    exitAction = new QAction(tr( "Exit"), this);
    exitAction->setObjectName("Exit");
    exitAction->setVisible(true);
    exitAction->setEnabled(true);
    exitAction->setIcon(QIcon(":/images/IconExit.png"));
}

void MainWindow::createConnections()
{
    connect(this, SIGNAL(signal_AddWarning(QString, eLogWarning)), warningJournal, SLOT(slot_addWarning(QString, eLogWarning)));
    connect(threadPoolAction, SIGNAL(triggered()), this, SLOT(slot_OnThreadPool()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(slot_OnExit()));
    connect(ui->pushButton_LongTask, SIGNAL(clicked()), this, SLOT(slot_CreateLongTask()));
    connect(ui->pushButton_RepeatTask, SIGNAL(clicked()), this, SLOT(slot_CreateRepeatTask()));
    connect(ui->pushButton_ShortTaskGenerator, SIGNAL(clicked()), this, SLOT(slot_ShortTaskGeneratorLaunch()));
    connect(ui->pushButton_LongTask_Stop, SIGNAL(clicked()), this, SLOT(slot_StopLongTask()));
    connect(ui->pushButton_RepeatTask_Stop, SIGNAL(clicked()), this, SLOT(slot_StopRepeatTask()));
    connect(ui->pushButton_ShortTaskGenerator_Stop, SIGNAL(clicked()), this, SLOT(slot_StopShortTaskGenerator()));
    //connect(cAdvThreadPool::getInstance().getEmitter(), SIGNAL())
}

void MainWindow::slot_OnThreadPool()
{
    pThreadPoolDialog->show();
}

void MainWindow::slot_OnExit()
{
    cAdvThreadPool::stopThreadPool();
    exit(0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    cAdvThreadPool::stopThreadPool();
    event->accept();
}

void MainWindow::slot_CreateLongTask()
{
    cleanLongFuturesIfNeed();

    QString text = ui->lineEdit_LongTaskName->text();

    if(!text.isEmpty())
    {
        QString longTaskName = QString("Long task '%1'").arg(text);
        auto it = std::find_if(pTaskContainer->m_longTaskFutures.begin(),
                        pTaskContainer->m_longTaskFutures.end(),
                        [longTaskName](std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>> _future)
                        {
                            if(_future->m_taskDescription == longTaskName)
                                return true;
                            else
                                return false;
                        });

        if(it == pTaskContainer->m_longTaskFutures.end())//task with this name absents
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
                pTaskContainer->m_longTaskFutures.push_back(pFuture);
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

    QString text = ui->lineEdit_LongTaskName->text();

    if(!text.isEmpty())
    {
        QString longTaskName = QString("Long task '%1'").arg(text);
        auto foundFuture = std::find_if(pTaskContainer->m_longTaskFutures.begin(),
            pTaskContainer->m_longTaskFutures.end(),
            [longTaskName](std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>> _future)
            {
                if(_future->m_taskDescription == longTaskName)
                    return true;
                else
                    return false;
            });

        if(foundFuture != pTaskContainer->m_longTaskFutures.end())//task with this name was found
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
    std::vector<std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>>> temp_vector(pTaskContainer->m_longTaskFutures.size());

    auto it = std::copy_if(pTaskContainer->m_longTaskFutures.begin(),
                           pTaskContainer->m_longTaskFutures.end(),
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
        pTaskContainer->m_longTaskFutures.remove_if(
            [](std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>> _future)
        {
            return _future->m_ready; //task was executed or not
        });

        temp_vector.clear();
    }
}


void MainWindow::slot_CreateRepeatTask()
{
    QString text = ui->lineEdit_RepeatTaskName->text();

    if(!text.isEmpty())
    {
        QString repeatTaskName = QString("Repeat task '%1'").arg(text);
        auto it = find_if(pTaskContainer->m_repeatTaskFutures.begin(),
                pTaskContainer->m_repeatTaskFutures.end(),
                [repeatTaskName](std::shared_ptr<cAdvFuture<qint32, cRepeatTask<Tasks, qint32>>> _future)
                {
                  if(_future->m_taskDescription == repeatTaskName)
                      return true;
                  else
                      return false;
                });

        if(it == pTaskContainer->m_repeatTaskFutures.end())//task with this name absents
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
                                                                pTaskContainer,\
                                                                Tasks,\
                                                                repeatTaskFunction,\
                                                                qint32,\
                                                                1000,\
                                                                repeatTaskName);

            if(future_of_repeatTask != nullptr)
            {
                pTaskContainer->m_repeatTaskFutures.push_back(future_of_repeatTask);
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
    QString text = ui->lineEdit_RepeatTaskName->text();

    if(!text.isEmpty())
    {
        QString repeatTaskName = QString("Repeat task '%1'").arg(text);
        auto foundFuture = find_if(
            pTaskContainer->m_repeatTaskFutures.begin(),
            pTaskContainer->m_repeatTaskFutures.end(),
            [repeatTaskName](std::shared_ptr<cAdvFuture<qint32, cRepeatTask<Tasks, qint32>>> _future)
            {
               if(_future->m_taskDescription == repeatTaskName)
                   return true;
               else
                   return false;
            });

        if(foundFuture == pTaskContainer->m_repeatTaskFutures.end())//task with this name was found
        {
            emit signal_AddWarning(QString("Error of stop: %1 don't exists").arg(repeatTaskName), eLogWarning::WARNING);
            return;
        }

        cAdvThreadPool::stopRepeatTask((*foundFuture)->m_repeatTaskID);

        //remove task from array
        pTaskContainer->m_repeatTaskFutures.remove_if(
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
    QString text = ui->lineEdit_ShortTaskQuantity->text();

    if(!text.isEmpty())
    {
        qint32 shortTaskQuantity = text.toInt();

        if(!pShortTaskGenerator->start(shortTaskQuantity))
        {
            emit signal_AddWarning(QString("Error: Start of Generator"), eLogWarning::WARNING);
        }
    }
    else
        emit signal_AddWarning(QString("Error: Start of Generator: bad value"), eLogWarning::WARNING);

}

void MainWindow::slot_StopShortTaskGenerator()
{
    pShortTaskGenerator->stopGenerator();
}
