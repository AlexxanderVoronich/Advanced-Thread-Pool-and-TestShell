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
class cAdvThreadPool
{
    friend class cAdvPoolGUI;
    friend class cCheckCoreWidget;
    friend class cAdvThread;

private://fields
    cThreadPoolSettings m_poolSettings;
    bool m_systemThreadRunnableSign = false;
    cLongTask<cAdvThreadPool, int> *m_systemTymerRunnable = nullptr;//system runnable for repeated tasks control
    adv_thread_ptr systemPoolThread;//thread for internal long task of pool

    std::vector<adv_thread_ptr> threadsArray;//array of pool's threads
    std::list<adv_thread_ptr> additionalThreadsArray;//additional array of pool's threads (for Long Tasks)
    std::mutex additionalThreadsMutex;

    std::vector<cRepeatedTaskAdapter> repeatedTaskArray;//array for repeated task (only for USE_REPEATED_TASK_MODE)
    std::mutex repeatTaskMutex;//mutex for repeat task array
    cAdvPoolEmitter* m_emitter = nullptr;//object-emitter for sending signals to GUI
    QElapsedTimer infoTimer;//timer of system thread for sending info to GUI

private://methods
    cAdvThreadPool();//( Mayers singletone)
    ~cAdvThreadPool();
    bool createThreadPool(int unsharedThreadsQuantity, int sharedThreadsQuantity);
    int startPoolMainFunction();
    void stopSystemTymerRunnable();
    int calculateCoreMask(int mask, int core_num, bool value);

public://methods
    static void startThreadPool(int unsharedThreadsQuantity,
                                int sharedThreadsQuantity,
                                cPoolModes poolMode = cPoolModes(),
                                QString file = "");
    static void stopThreadPool();
    static cAdvThreadPool& getInstance();
    static cThreadPoolSettings& getSettings();

   template<class _R, class _RUNNABLE>
    static std::shared_ptr<cAdvFuture<_R, _RUNNABLE>> launchRunnableObject(_RUNNABLE* _runObject, bool isSystemRunnable = false)
    {
        int threadNumber = -1;
        int runType = _runObject->getType();
        auto emitter = getInstance().getEmitter();
        adv_thread_ptr threadForTask = nullptr;

        if(!getSettings().m_isPoolStarted)
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
            std::shared_ptr<cAdvFuture<_R, _RUNNABLE> > futureData(new cAdvFuture<_R, _RUNNABLE>());
            runnable_closure runClosure = createRunClosure<_R, _RUNNABLE>(futureData, _runObject, threadNumber);

            threadForTask->appendRunnableTask(runClosure, runType);

            return futureData;
        }

        if(runType == eRunnableType::LONG_TASK)
        {
            threadForTask = getInstance().getFreeThread(threadNumber, runType);
            if(threadForTask == nullptr)
            {
                if(getSettings().m_poolModes.m_stretchMode == eStretchMode::NO_STRETCH)
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

        std::shared_ptr<cAdvFuture<_R, _RUNNABLE> > pFutureData(new cAdvFuture<_R, _RUNNABLE>());
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
            if(getSettings().m_poolModes.m_threadPoolMode == eThreadPoolMode::REPEATED_TASK_MODE)
            {
                int repeatTaskID = ++(getSettings().ID_TASK_COUNTER);
                if(getSettings().ID_TASK_COUNTER > 65000)
                    getSettings().ID_TASK_COUNTER = 0;

                cAdvThreadPool::getInstance().addRepeatedTask(runClosure, repeatTaskID, _runObject->getRepeatTime(), _runObject->getDescription());
                pFutureData->m_repeatTaskID = repeatTaskID;
            }
        }

        return pFutureData;
    }

    static bool stopRepeatTask(int taskID);

private://methods
    template<typename _R, class _RUNNABLE>
    static runnable_closure createRunClosure(std::shared_ptr<cAdvFuture<_R, _RUNNABLE>> pFutureData, _RUNNABLE* _runObject, int thread_id)
    {
        auto emitter = getInstance().getEmitter();
        std::function<typename _RUNNABLE::_RETURN_TYPE()> bind_run = std::bind(&_RUNNABLE::run, *_runObject);
        pFutureData->m_task = _runObject;
        pFutureData->m_hostObject = _runObject->getProcessor();

        //create and return the lambda with runnable object(task) inside its context
        return [thread_id, pFutureData, _runObject, bind_run, emitter](int _mode, int _value)->QString
        {
            switch(_mode)
            {
                case int(cAdvThread::eRUN_MODES::RUN):
                {
                    if(_runObject != nullptr)
                    {
                        pFutureData->m_taskDescription = _runObject->getDescription();
                        pFutureData->m_taskType = _runObject->getType();
                        pFutureData->m_ready = false;
                        if(pFutureData->m_taskType == eRunnableType::LONG_TASK)
                        {
                            emitter->sendSignal_Who_forUnsharedThread(thread_id, pFutureData->m_taskType, pFutureData->m_taskDescription);
                            int longTaskCounter = cAdvThreadPool::getInstance().getLongTaskQuantity();
                            emitter->sendSignal_longTaskQuantity(longTaskCounter);
                        }
                        else if(pFutureData->m_taskType == eRunnableType::LONG_TASK_EXTRA)
                        {
                            emitter->sendSignal_Who_forUnsharedThread(thread_id, pFutureData->m_taskType, pFutureData->m_taskDescription);
                            int longTaskCounter = cAdvThreadPool::getInstance().getLongTaskQuantity();
                            emitter->sendSignal_longTaskQuantity(longTaskCounter);
                        }

                        //call the task
                        pFutureData->m_data = bind_run();

                        if(pFutureData->m_taskType == eRunnableType::LONG_TASK)//delete wrapper for task (CRunnable) after task processing
                        {
                            delete _runObject;
                            int longTaskCounter = cAdvThreadPool::getInstance().getLongTaskQuantity();
                            --longTaskCounter;
                            emitter->sendSignal_longTaskQuantity(longTaskCounter);
                        }
                        else if(pFutureData->m_taskType == eRunnableType::LONG_TASK_EXTRA)//delete wrapper for task (CRunnable) after task processing
                        {
                            if(_runObject->getDeleteExtraThreadSign())
                                cAdvThreadPool::getInstance().deleteExtraThread(thread_id);
                            delete _runObject;
                        }
                        else if(pFutureData->m_taskType == eRunnableType::SHORT_TASK)//delete wrapper for task (CRunnable) after task processing
                        {
                            delete _runObject;
                        }
                        else if(pFutureData->m_taskType == eRunnableType::REPEAT_TASK)
                        {
                            if(pFutureData->m_data > 0)
                                cAdvThreadPool::getInstance().updateTimePeriodForRepeatTask(pFutureData->m_repeatTaskID, pFutureData->m_data);
                            else if(pFutureData->m_data == -1)
                            {
                                cAdvThreadPool::getInstance().stopRunnable_RepeatTask(pFutureData->m_repeatTaskID);
                                delete _runObject;
                            }
                        }

                        pFutureData->m_ready = true;
                    }
                    else
                    {
                        pFutureData->m_taskDescription = QString("None");
                    }
                    break;
                }

                case int(cAdvThread::eRUN_MODES::STOP):
                {
                    _runObject->stopRunnable();
                    break;
                }

                case int(cAdvThread::eRUN_MODES::STOP_WITHOUT_EXTRA_THREAD_DELETING):
                {
                    _runObject->setDeleteExtraThreadSign(false);
                    _runObject->stopRunnable();
                    break;
                }

                case int(cAdvThread::eRUN_MODES::WHO):
                {
                    pFutureData->m_taskDescription = _runObject->getDescription();
                    return pFutureData->m_taskDescription;
                }

                case int(cAdvThread::eRUN_MODES::RUN_TYPE):
                {
                    pFutureData->m_taskType = _runObject->getType();
                    return QString("%1").arg(pFutureData->m_taskType);
                }

                case int(cAdvThread::eRUN_MODES::GET_COUNTER):
                {
                    return QString::number(pFutureData->m_executionCounter);
                }

                case int(cAdvThread::eRUN_MODES::DECREASE_COUNTER):
                {
                    --(pFutureData->m_executionCounter);
                    return QString::number(pFutureData->m_executionCounter);
                }

                case int(cAdvThread::eRUN_MODES::START_TIMER_FOR_INTERVAL):
                {
                    pFutureData->m_timeIntervalForHoldOverShortTask = _value;
                    pFutureData->m_qt_timer.start();
                    break;
                }

                case int(cAdvThread::eRUN_MODES::IS_TIMER_OVER):
                {
                    if(pFutureData->m_timeIntervalForHoldOverShortTask == 0)
                    {
                        pFutureData->m_resultIsTimerOver = true;
                        return QString::number(1);
                    }
                    else if(pFutureData->m_qt_timer.isValid())
                    {
                        int timeout = pFutureData->m_qt_timer.elapsed();
                        if(timeout >= pFutureData->m_timeIntervalForHoldOverShortTask)
                        {
                            pFutureData->m_resultIsTimerOver = true;
                            return QString::number(1);
                        }
                        else
                        {
                            pFutureData->m_resultIsTimerOver = false;
                            return QString::number(0);
                        }
                    }
                }
            }
            return pFutureData->m_taskDescription;
        };
    }

    adv_thread_ptr getFreeThread(int &_thread_id, int _runType);
    adv_thread_ptr createSystemPoolThread();
    void stopAll();
    void waitAll();
    void deleteExtraThread(int _id, bool _sign = false);
    cAdvPoolEmitter* getEmitter(){return m_emitter;}
    void requestState();
    void addRepeatedTask(runnable_closure _task, int _id, int _timePeriod, QString _who);
    bool stopRunnable_RepeatTask(int _taskID);
    bool updateTimePeriodForRepeatTask(int _taskID, int _newTimePeriod);
    void saveSettings(int _unsharedQ, int _sharedQ, eAffinityMode _affinityMode);
    void changeAffinityMask(int _threadID, int _coreID, bool _state);
    //int setMainThreadAffinity(int mask);
    int getLongTaskQuantity();
    adv_thread_ptr createAdditionalNotSharedThread(int &_id);
};

#endif // CWSSTHREADPOOL_H
