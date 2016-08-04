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

    warningJournal = new WarningJournal(this, QString("Notification box"));
    addDockWidget(Qt::BottomDockWidgetArea, warningJournal->warningDockWidget);
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(warningJournal->warningDockWidget->toggleViewAction());

    CPoolModes modes;
    modes.threadPoolMode = eThreadPoolMode::REPEATED_TASK_MODE;
    modes.stretchMode = eStretchMode::YES_STRETCH;
    CAdvThreadPool::startThreadPool(2, 2, modes, "../etc/ThreadPoolSettings.ini");

    pTaskContainer = new Tasks();
    pShortTaskGenerator = new ShortTaskGenerator();
    createActions();
    createConnections();

    pToolBar = addToolBar(tr( "Object Operations" ));
    addToolBar(Qt::TopToolBarArea, pToolBar);

    pToolBar->addAction(threadPoolAction);
    pToolBar->addAction(exitAction);

    pThreadPoolDialog = new CAdvPoolGUI(this);
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
    connect(this, SIGNAL(signal_AddWarning(QString, eLogWarning)), warningJournal, SLOT(slot_AddWarning(QString, eLogWarning)));
    connect(threadPoolAction, SIGNAL(triggered()), this, SLOT(slot_OnThreadPool()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(slot_OnExit()));
    connect(ui->pushButton_LongTask, SIGNAL(clicked()), this, SLOT(slot_CreateLongTask()));
    connect(ui->pushButton_RepeatTask, SIGNAL(clicked()), this, SLOT(slot_CreateRepeatTask()));
    connect(ui->pushButton_ShortTaskGenerator, SIGNAL(clicked()), this, SLOT(slot_ShortTaskGeneratorLaunch()));
    connect(ui->pushButton_LongTask_Stop, SIGNAL(clicked()), this, SLOT(slot_StopLongTask()));
    connect(ui->pushButton_RepeatTask_Stop, SIGNAL(clicked()), this, SLOT(slot_StopRepeatTask()));
    connect(ui->pushButton_ShortTaskGenerator_Stop, SIGNAL(clicked()), this, SLOT(slot_StopShortTaskGenerator()));
    //connect(CAdvThreadPool::getInstance().getEmitter(), SIGNAL())
}

void MainWindow::slot_OnThreadPool()
{
    pThreadPoolDialog->show();
}

void MainWindow::slot_OnExit()
{
    CAdvThreadPool::stopThreadPool();
    exit(0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    CAdvThreadPool::stopThreadPool();
    event->accept();
}

void MainWindow::slot_CreateLongTask()
{
    cleanLongFuturesIfNeed();

    QString text = ui->lineEdit_LongTaskName->text();

    if(!text.isEmpty())
    {
        QString longTaskName = QString("Long task '%1'").arg(text);
        auto it = find_if(pTaskContainer->longTaskFutures.begin(),
                        pTaskContainer->longTaskFutures.end(),
                        [longTaskName](std::shared_ptr<CAdvFuture<qint32, CLongTask<LongTaskContainer, qint32>>> _future)
                        {
                            if(_future->taskDescription == longTaskName)
                                return true;
                            else
                                return false;
                        });

        if(it == pTaskContainer->longTaskFutures.end())//task with this name absents
        {
            //create object with task-function
            LongTaskContainer* longTaskObject = new LongTaskContainer(longTaskName);
            //std::cout<<"ptr to longTaskObject = "<<longTaskObject<<std::endl;
            //create new long task
            auto pLongTask = new CLongTask<LongTaskContainer, qint32>(longTaskObject,
                                                    &LongTaskContainer::longTaskFunction,
                                                    &LongTaskContainer::stopLongTaskFunction,
                                                    longTaskName);

            auto pFuture = CAdvThreadPool::launchRunnableObject<qint32, CLongTask<LongTaskContainer, qint32>>(pLongTask);
            if(pFuture != nullptr)
            {
                pTaskContainer->longTaskFutures.push_back(pFuture);
                emit signal_AddWarning(QString("Create %1").arg(longTaskName), eLogWarning::message);
            }
            else
                emit signal_AddWarning(QString("Error of creation: no free unshared thread"), eLogWarning::warning);

        }
        else //task already exists
        {
            emit signal_AddWarning(QString("Error of creation: %1 already exists").arg(longTaskName), eLogWarning::warning);
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
        auto foundFuture = find_if(pTaskContainer->longTaskFutures.begin(),
            pTaskContainer->longTaskFutures.end(),
            [longTaskName](std::shared_ptr<CAdvFuture<qint32, CLongTask<LongTaskContainer, qint32>>> _future)
            {
                if(_future->taskDescription == longTaskName)
                    return true;
                else
                    return false;
            });

        if(foundFuture != pTaskContainer->longTaskFutures.end())//task with this name was found
        {
            (*foundFuture)->pTask->stopRunnable();//stop long task
            emit signal_AddWarning(QString("Stop %1").arg(longTaskName), eLogWarning::message);

        }
        else
        {
            emit signal_AddWarning(QString("Error of stop: %1 don't exists").arg(longTaskName), eLogWarning::warning);
        }
    }
}

void MainWindow::cleanLongFuturesIfNeed()
{
    std::vector<std::shared_ptr<CAdvFuture<qint32, CLongTask<LongTaskContainer, qint32>>>> temp_vector(pTaskContainer->longTaskFutures.size());

    auto it = std::copy_if(pTaskContainer->longTaskFutures.begin(),
                           pTaskContainer->longTaskFutures.end(),
                           temp_vector.begin(),
                           [](std::shared_ptr<CAdvFuture<qint32, CLongTask<LongTaskContainer, qint32>>> _future)
                           {
                               return _future->ready; //task was executed or not
                           });

    temp_vector.resize(std::distance(temp_vector.begin(),it));  // shrink container to new size

    for(auto temp_future: temp_vector)
    {
        QString longTaskName = temp_future->taskDescription;
        emit signal_AddWarning(QString("Delete object what incapsulate %1").arg(longTaskName), eLogWarning::warning);

        LongTaskContainer* ptr = static_cast<LongTaskContainer*>(temp_future->pHostObject);

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
        pTaskContainer->longTaskFutures.remove_if(
            [](std::shared_ptr<CAdvFuture<qint32, CLongTask<LongTaskContainer, qint32>>> _future)
        {
            return _future->ready; //task was executed or not
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
        auto it = find_if(pTaskContainer->repeatTaskFutures.begin(),
                pTaskContainer->repeatTaskFutures.end(),
                [repeatTaskName](std::shared_ptr<CAdvFuture<qint32, CRepeatTask<Tasks, qint32>>> _future)
                {
                  if(_future->taskDescription == repeatTaskName)
                      return true;
                  else
                      return false;
                });

        if(it == pTaskContainer->repeatTaskFutures.end())//task with this name absents
        {
            //1)create new repeat task (usual variant)
/*            auto repeatTask = new CRepeatTask<Tasks, qint32>(pTaskContainer,
                                                    &Tasks::repeatTaskFunction,
                                                    1000,
                                                    repeatTaskName);
            auto future_of_repeatTask = CAdvThreadPool::launchRunnableObject<qint32, CRepeatTask<Tasks, qint32>>(repeatTask);
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
                pTaskContainer->repeatTaskFutures.push_back(future_of_repeatTask);
                emit signal_AddWarning(QString("Create %1").arg(repeatTaskName), eLogWarning::message);
            }
            else
                emit signal_AddWarning(QString("Error of repeat task launch"), eLogWarning::warning);

        }
        else //task already exists
        {
            emit signal_AddWarning(QString("Error of creation: %1 already exists").arg(repeatTaskName), eLogWarning::warning);
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
            pTaskContainer->repeatTaskFutures.begin(),
            pTaskContainer->repeatTaskFutures.end(),
            [repeatTaskName](std::shared_ptr<CAdvFuture<qint32, CRepeatTask<Tasks, qint32>>> _future)
            {
               if(_future->taskDescription == repeatTaskName)
                   return true;
               else
                   return false;
            });

        if(foundFuture == pTaskContainer->repeatTaskFutures.end())//task with this name was found
        {
            emit signal_AddWarning(QString("Error of stop: %1 don't exists").arg(repeatTaskName), eLogWarning::warning);
            return;
        }

        CAdvThreadPool::stopRepeatTask((*foundFuture)->repeatTaskID);

        //remove task from array
        pTaskContainer->repeatTaskFutures.remove_if(
        [repeatTaskName](std::shared_ptr<CAdvFuture<qint32, CRepeatTask<Tasks, qint32>>> _future)
        {
            if(_future->taskDescription == repeatTaskName)
                return true;
            else
                return false;
        });

        emit signal_AddWarning(QString("Stop %1").arg(repeatTaskName), eLogWarning::message);
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
            emit signal_AddWarning(QString("Error: Start of Generator"), eLogWarning::warning);
        }
    }
    else
        emit signal_AddWarning(QString("Error: Start of Generator: bad value"), eLogWarning::warning);

}

void MainWindow::slot_StopShortTaskGenerator()
{
    pShortTaskGenerator->stopGenerator();
}
