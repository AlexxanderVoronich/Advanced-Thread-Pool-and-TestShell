// FILE AdvThread.cpp
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "AdvThread.h"
#include "AdvThreadPool.h"
#include <QtGlobal>
#include <QtDebug>

CAdvThread::CAdvThread(int a_iThreadNumber, int a_iCoreQuantity)
    :isThreadWork(true),taskArray()
    ,nativeThread(&CAdvThread::threadMainFunction, this)
    ,currentRunnableClosure(nullptr)
{
    threadNumber = a_iThreadNumber;
    coreQuantity = a_iCoreQuantity;
    qDebug() << "CAdvThread constructor for thread_id = " << threadNumber;
}

CAdvThread::~CAdvThread()
{
    qDebug() << "~CAdvThread" << threadNumber << "destructor begin";
    isThreadWork = false;
    conditionVariable.notify_one();
    if(nativeThread.joinable())
    {
        qDebug() << "~CAdvThread - joinable";
        nativeThread.join();
    }

    qDebug() << "~CAdvThread" << threadNumber << "destructor end";
}

bool CAdvThread::appendRunnableTask(runnable_closure run, int run_type)
{
    if((run_type == eRunnableType::LONG_TASK || run_type == eRunnableType::LONG_TASK_EXTRA) && currentRunnableClosure)
        return false;

    if(run_type == eRunnableType::LONG_TASK_EXTRA)
        qDebug() << "LONG_TASK_EXTRA append to thread's queue";

    std::unique_lock<std::mutex> locker(threadMutex);
    taskArray.push(run);
    conditionVariable.notify_one();

    auto currentTime = std::chrono::high_resolution_clock::now();

    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTimeOfTaskQuantityAnalize).count();
    if(delta >= 1000)
    {
        lastTimeOfTaskQuantityAnalize = currentTime;

        if(run_type == eRunnableType::SHORT_TASK || run_type == eRunnableType::REPEAT_TASK)
        {
            CAdvThreadPool::getInstance().getEmitter()->sendSignal_MeanCountTasks(getThreadNumber(),
                                QString("%1/%2").arg(averageTaskQuantity).arg(taskArray.size()));
        }

        averageTaskQuantity = 0;
    }

    averageTaskQuantity++;

    return true;
}

size_t CAdvThread::getTaskCount()
{
    std::unique_lock<std::mutex> locker(threadMutex);
    return taskArray.size();
}


size_t CAdvThread::getAllTaskCount()
{
    std::unique_lock<std::mutex> locker(threadMutex);
    return (currentRunnableClosure)? taskArray.size()+1: taskArray.size();
}

std::chrono::system_clock::time_point CAdvThread::getLastTimeOfTaskLaunch() const
{
    return lastTimeOfTaskLaunch;
}

bool CAdvThread::isEmpty()
{
    std::unique_lock<std::mutex> locker(threadMutex);
    return taskArray.empty()&&!currentRunnableClosure;
}

void CAdvThread::getCurrentHandle()
{
#ifdef Q_OS_LINUX
    //cpu_set_t cpuset;
    threadHandle = pthread_self();
    CPU_ZERO(&cpuset);
#else
    thread_id = GetCurrentThreadId();
    //thread_handle = GetCurrentThread();
    threadHandle = OpenThread(THREAD_ALL_ACCESS, false, thread_id);
#endif

}

bool CAdvThread::setAffinity()
{
#ifdef Q_OS_LINUX
    if(threadHandle != 0)
    {
        int j;
        for (j = 0; j < coreQuantity; j++)
            if(affinityData.coreMask & (1<<j))
                CPU_SET(j, &cpuset);
        pthread_setaffinity_np(threadHandle, sizeof(cpu_set_t), &cpuset);
    }
#else
    if(threadHandle != 0)
    {
        DWORD_PTR res = SetThreadAffinityMask(threadHandle, affinityData.coreMask);
        if (0 == res)
        {
         // failure
        }
    }
#endif

     affinityData.coreChanged = 0;
}

void CAdvThread:: wait()
{
    if(nativeThread.joinable())
        nativeThread.join();
     currentRunnableClosure = nullptr;
}

void CAdvThread::stop()
{
    if(currentRunnableClosure)
        currentRunnableClosure(int(eRUN_MODES::STOP), 0);//StopRunnable

    isThreadWork = false;
    conditionVariable.notify_one();
}

void CAdvThread::stopWithoutDeleteExtraThread()
{
    if(currentRunnableClosure)
        currentRunnableClosure(int(eRUN_MODES::STOP_WITHOUT_EXTRA_THREAD_DELETING), 0);//StopRunnable

    isThreadWork = false;
    conditionVariable.notify_one();
}


QString CAdvThread::getWho()
{
    QString who;
    if(currentRunnableClosure)
        who = currentRunnableClosure(int(eRUN_MODES::WHO), 0);
    else
        who = QString("empty");

    return who;
}

int CAdvThread::getRunType()
{
    int runType;
    if(currentRunnableClosure)
        runType = currentRunnableClosure(int(eRUN_MODES::RUN_TYPE), 0).toInt();
    else
        runType = 0;

    return runType;
}

bool CAdvThread::setCoreMask(int mask)
{
    affinityData.coreMask = mask;
    affinityData.coreChanged = 1;
    setAffinity();
}


void CAdvThread::threadMainFunction()
{
    getCurrentHandle();
    setAffinity();
    while (isThreadWork)
    {
        std::unique_lock<std::mutex> locker(threadMutex);
        // wait notification and check that it does not a false awakening
        // Thread must awake if list is not empty or it was poweroff
        conditionVariable.wait(locker, [&](){ return !taskArray.empty() || !isThreadWork || affinityData.coreChanged;});

        while(!taskArray.empty())
        {
            if(!isThreadWork)//if stop thread of pool
            {
                while(!taskArray.empty())//stop all tasks
                {
                    runnable_closure runClosure = taskArray.front();//get task from array
                    taskArray.pop();
                    locker.unlock();//unlock before task call
                    runClosure(int(eRUN_MODES::STOP), 0);
                    locker.lock();
                }
                return;
            }

            currentRunnableClosure = taskArray.front();//get task from array
            taskArray.pop();

            locker.unlock();//unlock before task call
            try
            {
                if(threadType == eThreadType::THREAD_SHARED && getRunType() == eRunnableType::SHORT_TASK)
                {
                    QString stringTimerResult = currentRunnableClosure(int(eRUN_MODES::IS_TIMER_OVER), 0);
                    if(stringTimerResult == QString("1"))
                    {
                        lastTimeOfTaskLaunch = std::chrono::high_resolution_clock::now();
                        isReadyForSend_WarningAboutShortTaskFreeze = true;
                        currentRunnableClosure(int(eRUN_MODES::RUN), 0);
                        isReadyForSend_WarningAboutShortTaskFreeze = false;
                    }
                    else //task hold over
                        appendRunnableTask(currentRunnableClosure, eRunnableType::SHORT_TASK);
                }
                else
                {
                    lastTimeOfTaskLaunch = std::chrono::high_resolution_clock::now();
                    isReadyForSend_WarningAboutShortTaskFreeze = true;
                    currentRunnableClosure(int(eRUN_MODES::RUN), 0);
                    isReadyForSend_WarningAboutShortTaskFreeze = false;
                }

            }
            catch(holdOverTask_Exception action)
            {
                if(threadType == eThreadType::THREAD_SHARED && getRunType() == eRunnableType::SHORT_TASK)
                {
                    int counterOfExecution = currentRunnableClosure(int(eRUN_MODES::GET_COUNTER), 0).toInt();
                    if(counterOfExecution>0)
                    {
                        CAdvThreadPool::getInstance().getEmitter()->addWarningToShell(QString("Task was hold over - counter = %1 (%2)").arg(counterOfExecution)
                                                                                      .arg(QString::fromStdString(action.what())),
                                                                                      eLogWarning::message);
                        currentRunnableClosure(int(eRUN_MODES::DECREASE_COUNTER), 0);
                        currentRunnableClosure(int(eRUN_MODES::START_TIMER_FOR_INTERVAL), action.getInterval());
                        appendRunnableTask(currentRunnableClosure, eRunnableType::SHORT_TASK);
                    }
                    else
                    {
                        CAdvThreadPool::getInstance().getEmitter()->addWarningToShell(QString("Task will not processing - counter over (%2)").arg(counterOfExecution)
                                                                                      .arg(QString::fromStdString(action.what())),
                                                                                      eLogWarning::warning);
                    }
                }

            }
            catch(std::exception& e)
            {
                QString temp = QString("%1 - CAdvThread::threadMainFunction - ").arg(e.what());
                temp += getWho();
                std::cout<<temp.toStdString()<<std::endl;
                CAdvThreadPool::getInstance().getEmitter()->addWarningToShell(temp, eLogWarning::warning);
            }
            catch(...)
            {               
                QString temp = QString("Undefined exception - CAdvThread::threadMainFunction - ");
                temp += getWho();
                std::cout<<temp.toStdString()<<std::endl;
                CAdvThreadPool::getInstance().getEmitter()->addWarningToShell(temp, eLogWarning::warning);
            }

            locker.lock(); // lock before m_TaskArray.empty()
            currentRunnableClosure = nullptr;

            if(threadType == eThreadType::THREAD_NOT_SHARED)
                CAdvThreadPool::getInstance().getEmitter()->sendSignal_DeleteLongTask(getThreadNumber());
            else if(threadType == eThreadType::THREAD_NOT_SHARED_EXTRA)
                CAdvThreadPool::getInstance().getEmitter()->sendSignal_DeleteExtraLongTask(getThreadNumber());
        }
    }
}
