// FILE AdvThreadPool.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef CADVTHREADPOOL_H
#define CADVTHREADPOOL_H

#include "Runnable.h"
#include "AdvThread.h"
#include "AdvPoolGUI.h"
#include "ThreadPoolSettings.h"
#include "ServiceStructures.h"
#include "AdvPoolEmitter.h"

#include <functional>
#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <QObject>
#include <QtDebug>

/////////////////////////////////
class CAdvThreadPool
{
    friend class CAdvPoolGUI;
    friend class CheckCoreWidget;
    friend class CAdvThread;
private://fields
    CThreadPoolSettings poolSettings;
    bool systemThreadRunnableSign = false;
    CLongTask<CAdvThreadPool, int> *systemTymerRunnable = nullptr;//system runnable for repeated tasks control
    adv_thread_ptr systemPoolThread;//thread for internal long task of pool

    std::vector<adv_thread_ptr> threadsArray;//array of pool's threads
    std::list<adv_thread_ptr> additionalThreadsArray;//additional array of pool's threads (for Long Tasks)
    std::mutex additionalThreadsMutex;

    std::vector<CRepeatedTaskAdapter> repeatedTaskArray;//array for repeated task (only for USE_REPEATED_TASK_MODE)
    std::mutex repeatTaskMutex;//mutex for repeat task array
    CAdvPoolEmitter* emitter = nullptr;//object-emitter for sending signals to GUI
    QElapsedTimer infoTimer;//timer of system thread for sending info to GUI

private://methods
    CAdvThreadPool();//( Mayers singletone)
    ~CAdvThreadPool();
    bool createThreadPool(int unsharedThreadsQuantity, int sharedThreadsQuantity);
    int startPoolMainFunction();
    void stopSystemTymerRunnable();
    int calculateCoreMask(int mask, int core_num, bool value);

public://methods
    static void startThreadPool(int unsharedThreadsQuantity,
                                int sharedThreadsQuantity,
                                CPoolModes poolMode = CPoolModes(),
                                QString file = "");
    static void stopThreadPool();
    static CAdvThreadPool& getInstance();
    static CThreadPoolSettings& getSettings();

   template<class _R, class _RUNNABLE>
    static std::shared_ptr<CAdvFuture<_R, _RUNNABLE>> launchRunnableObject(_RUNNABLE* _runObject, bool isSystemRunnable = false)
    {
        int threadNumber = -1;
        int runType = _runObject->getType();
        auto emitter = getInstance().getEmitter();
        adv_thread_ptr threadForTask = nullptr;

        if(!getSettings().isPoolStarted)
        {
            emitter->sendSignal_poolNotWork();
            return nullptr;
        }

        if(isSystemRunnable)//start of system pool thread
        {
            if(runType != eRunnableType::LONG_TASK)
            {
                qDebug() << "Error: task is not Long (for system pool thread)";
                return nullptr;
            }

            threadForTask = getInstance().createSystemPoolThread();//thread with number == 0
            if(threadForTask == nullptr)
            {
                qDebug() << "System pool thread already exists";
                return nullptr;
            }

            threadNumber = threadForTask->getThreadNumber();
            std::shared_ptr<CAdvFuture<_R, _RUNNABLE> > pFutureData(new CAdvFuture<_R, _RUNNABLE>());
            runnable_closure runClosure = createRunClosure<_R, _RUNNABLE>(pFutureData, _runObject, threadNumber);

            threadForTask->appendRunnableTask(runClosure, runType);

            return pFutureData;
        }

        if(runType == eRunnableType::LONG_TASK)
        {
            threadForTask = getInstance().getFreeThread(threadNumber, runType);
            if(threadForTask == nullptr)
            {
                if(getSettings().poolModes.stretchMode == eStretchMode::NO_STRETCH)
                {
                    emitter->sendSignal_NoThread();
                    return nullptr;
                }
                else
                {
                    threadForTask = getInstance().createAdditionalNotSharedThread(threadNumber);
                    if(threadForTask == nullptr)
                    {
                        emitter->sendSignal_NoThread();
                        return nullptr;
                    }
                    else if(!_runObject->updateLongTypeToExtra())
                    {
                        emitter->sendSignal_NoThread();
                        return nullptr;
                    }

                    runType = _runObject->getType();
                }
            }
        }
        else if(runType == eRunnableType::SHORT_TASK)
        {
            threadForTask = getInstance().getFreeThread(threadNumber, runType);
            if(threadForTask == nullptr)
            {
                emitter->sendSignal_NoThread();
                return nullptr;
            }
        }

        std::shared_ptr<CAdvFuture<_R, _RUNNABLE> > pFutureData(new CAdvFuture<_R, _RUNNABLE>());
        runnable_closure runClosure = createRunClosure<_R, _RUNNABLE>(pFutureData, _runObject, threadNumber);

        if(runType == eRunnableType::LONG_TASK || runType == eRunnableType::LONG_TASK_EXTRA)
        {
            threadForTask->appendRunnableTask(runClosure, runType);
        }
        else if(runType == eRunnableType::SHORT_TASK)
        {
            threadForTask->appendRunnableTask(runClosure, runType);
        }
        else if(runType == eRunnableType::REPEAT_TASK)
        {
            if(getSettings().poolModes.threadPoolMode == eThreadPoolMode::REPEATED_TASK_MODE)
            {
                int repeatTaskID = ++(getSettings().ID_TASK_COUNTER);
                if(getSettings().ID_TASK_COUNTER > 65000)
                    getSettings().ID_TASK_COUNTER = 0;

                CAdvThreadPool::getInstance().addRepeatedTask(runClosure, repeatTaskID, _runObject->getRepeatTime(), _runObject->getDescription());
                pFutureData->repeatTaskID = repeatTaskID;
            }
        }

        return pFutureData;
    }

    static bool stopRepeatTask(int taskID);

private://methods
    template<typename _R, class _RUNNABLE>
    static runnable_closure createRunClosure(std::shared_ptr<CAdvFuture<_R, _RUNNABLE>> pFutureData, _RUNNABLE* _runObject, int thread_id)
    {
        auto emitter = getInstance().getEmitter();
        std::function<typename _RUNNABLE::_RETURN_TYPE()> bind_run = std::bind(&_RUNNABLE::run, *_runObject);
        pFutureData->pTask = _runObject;
        pFutureData->pHostObject = _runObject->getProcessor();

        //create and return the lambda with runnable object(task) inside its context
        return [thread_id, pFutureData, _runObject, bind_run, emitter](int MODE, int VALUE)->QString
        {
            switch(MODE)
            {
                case int(CAdvThread::eRUN_MODES::RUN):
                {
                    if(_runObject != nullptr)
                    {
                        pFutureData->taskDescription = _runObject->getDescription();
                        pFutureData->taskType = _runObject->getType();
                        pFutureData->ready = false;
                        if(pFutureData->taskType == eRunnableType::LONG_TASK)
                        {
                            emitter->sendSignal_Who_forUnsharedThread(thread_id, pFutureData->taskType, pFutureData->taskDescription);
                            int longTaskCounter = CAdvThreadPool::getInstance().getLongTaskQuantity();
                            emitter->sendSignal_longTaskQuantity(longTaskCounter);
                        }
                        else if(pFutureData->taskType == eRunnableType::LONG_TASK_EXTRA)
                        {
                            emitter->sendSignal_Who_forUnsharedThread(thread_id, pFutureData->taskType, pFutureData->taskDescription);
                            int longTaskCounter = CAdvThreadPool::getInstance().getLongTaskQuantity();
                            emitter->sendSignal_longTaskQuantity(longTaskCounter);
                        }

                        //call the task
                        pFutureData->data = bind_run();

                        if(pFutureData->taskType == eRunnableType::LONG_TASK)//delete wrapper for task (CRunnable) after task processing
                        {
                            delete _runObject;
                            int longTaskCounter = CAdvThreadPool::getInstance().getLongTaskQuantity();
                            --longTaskCounter;
                            emitter->sendSignal_longTaskQuantity(longTaskCounter);
                        }
                        else if(pFutureData->taskType == eRunnableType::LONG_TASK_EXTRA)//delete wrapper for task (CRunnable) after task processing
                        {
                            if(_runObject->getDeleteExtraThreadSign())
                                CAdvThreadPool::getInstance().deleteExtraThread(thread_id);
                            delete _runObject;
                        }
                        else if(pFutureData->taskType == eRunnableType::SHORT_TASK)//delete wrapper for task (CRunnable) after task processing
                        {
                            delete _runObject;
                        }
                        else if(pFutureData->taskType == eRunnableType::REPEAT_TASK)
                        {
                            if(pFutureData->data > 0)
                                CAdvThreadPool::getInstance().updateTimePeriodForRepeatTask(pFutureData->repeatTaskID, pFutureData->data);
                            else if(pFutureData->data == -1)
                            {
                                CAdvThreadPool::getInstance().stopRunnable_RepeatTask(pFutureData->repeatTaskID);
                                delete _runObject;
                            }
                        }

                        pFutureData->ready = true;
                    }
                    else
                    {
                        pFutureData->taskDescription = QString("None");
                    }
                    break;
                }

                case int(CAdvThread::eRUN_MODES::STOP):
                {
                    _runObject->stopRunnable();
                    break;
                }

                case int(CAdvThread::eRUN_MODES::STOP_WITHOUT_EXTRA_THREAD_DELETING):
                {
                    _runObject->setDeleteExtraThreadSign(false);
                    _runObject->stopRunnable();
                    break;
                }

                case int(CAdvThread::eRUN_MODES::WHO):
                {
                    pFutureData->taskDescription = _runObject->getDescription();
                    return pFutureData->taskDescription;
                }

                case int(CAdvThread::eRUN_MODES::RUN_TYPE):
                {
                    pFutureData->taskType = _runObject->getType();
                    return QString("%1").arg(pFutureData->taskType);
                }

                case int(CAdvThread::eRUN_MODES::GET_COUNTER):
                {
                    return QString::number(pFutureData->executionCounter);
                }

                case int(CAdvThread::eRUN_MODES::DECREASE_COUNTER):
                {
                    --(pFutureData->executionCounter);
                    return QString::number(pFutureData->executionCounter);
                }

                case int(CAdvThread::eRUN_MODES::START_TIMER_FOR_INTERVAL):
                {
                    pFutureData->timeIntervalForHoldOverShortTask = VALUE;
                    pFutureData->qt_timer.start();
                    break;
                }

                case int(CAdvThread::eRUN_MODES::IS_TIMER_OVER):
                {
                    if(pFutureData->timeIntervalForHoldOverShortTask == 0)
                    {
                        pFutureData->resultIsTimerOver = true;
                        return QString::number(1);
                    }
                    else if(pFutureData->qt_timer.isValid())
                    {
                        int timeout = pFutureData->qt_timer.elapsed();
                        if(timeout >= pFutureData->timeIntervalForHoldOverShortTask)
                        {
                            pFutureData->resultIsTimerOver = true;
                            return QString::number(1);
                        }
                        else
                        {
                            pFutureData->resultIsTimerOver = false;
                            return QString::number(0);
                        }
                    }
                }
            }
            return pFutureData->taskDescription;
        };
    }

    adv_thread_ptr getFreeThread(int &thread_id, int runType);
    adv_thread_ptr createSystemPoolThread();
    void stopAll();
    void waitAll();
    void deleteExtraThread(int id, bool sign = false);
    CAdvPoolEmitter* getEmitter(){return emitter;}
    void requestState();
    void addRepeatedTask(runnable_closure task, int ID, int timePeriod, QString who);
    bool stopRunnable_RepeatTask(int taskID);
    bool updateTimePeriodForRepeatTask(int taskID, int newTimePeriod);
    void saveSettings(int unsharedQ, int sharedQ, eAffinityMode affinityMode);
    void changeAffinityMask(int threadID, int coreID, bool state);
    //int setMainThreadAffinity(int mask);
    int getLongTaskQuantity();
    adv_thread_ptr createAdditionalNotSharedThread(int &id);
};

#endif // CWSSTHREADPOOL_H
