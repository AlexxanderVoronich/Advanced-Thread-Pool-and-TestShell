// FILE AdvThread.cpp
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "AdvThread.h"
#include "AdvThreadPool.h"
#include <QtGlobal>
#include <QtDebug>

cAdvThread::cAdvThread(int _threadNumber, int _coreQuantity)
    :m_isThreadWork(true),m_taskArray()
    ,m_nativeThread(&cAdvThread::threadMainFunction, this)
    ,m_currentRunnableClosure(nullptr)
{
    m_threadNumber = _threadNumber;
    m_coreQuantity = _coreQuantity;
    qDebug() << "CAdvThread constructor for thread_id = " << m_threadNumber;
}

cAdvThread::~cAdvThread()
{
    qDebug() << "~CAdvThread" << m_threadNumber << "destructor begin";
    m_isThreadWork = false;
    m_conditionVariable.notify_one();
    if(m_nativeThread.joinable())
    {
        qDebug() << "~CAdvThread - joinable";
        m_nativeThread.join();
    }

    qDebug() << "~CAdvThread" << m_threadNumber << "destructor end";
}

bool cAdvThread::appendRunnableTask(runnable_closure _run, int _runType)
{
    if((_runType == eRunnableType::LONG_TASK || _runType == eRunnableType::LONG_TASK_EXTRA) && m_currentRunnableClosure)
        return false;

    if(_runType == eRunnableType::LONG_TASK_EXTRA)
        qDebug() << "LONG_TASK_EXTRA append to thread's queue";

    std::unique_lock<std::mutex> locker(m_threadMutex);
    m_taskArray.push(_run);
    m_conditionVariable.notify_one();

    std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();

    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastTimeOfTaskQuantityAnalize).count();
    if(delta >= 1000)
    {
        m_lastTimeOfTaskQuantityAnalize = currentTime;

        if(_runType == eRunnableType::SHORT_TASK || _runType == eRunnableType::REPEAT_TASK)
        {
            cAdvThreadPool::getInstance().getEmitter()->sendSignal_AverageQuantityOfTasks(getThreadNumber(),
                                QString("%1/%2").arg(m_averageTaskQuantity).arg(m_taskArray.size()));
        }

        m_averageTaskQuantity = 0;
    }

    m_averageTaskQuantity++;

    return true;
}

size_t cAdvThread::getTaskCount()
{
    std::unique_lock<std::mutex> locker(m_threadMutex);
    return m_taskArray.size();
}


size_t cAdvThread::getAllTaskCount()
{
    std::unique_lock<std::mutex> locker(m_threadMutex);
    return (m_currentRunnableClosure)? m_taskArray.size()+1: m_taskArray.size();
}

/*std::chrono::system_clock::time_point CAdvThread::getLastTimeOfTaskLaunch() const
{
    return lastTimeOfTaskLaunch;
}*/

std::chrono::time_point<std::chrono::high_resolution_clock> cAdvThread::getLastTimeOfTaskLaunch() const
{
    return m_lastTimeOfTaskLaunch;
}

bool cAdvThread::isEmpty()
{
    std::unique_lock<std::mutex> locker(m_threadMutex);
    return m_taskArray.empty()&&!m_currentRunnableClosure;
}

void cAdvThread::getCurrentHandle()
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

void cAdvThread::setAffinity()
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
        DWORD_PTR res = SetThreadAffinityMask(threadHandle, m_affinityData.m_coreMask);
        if (0 == res)
        {
         // failure
        }
    }
#endif

     m_affinityData.m_coreChanged = 0;
}

void cAdvThread:: wait()
{
    if(m_nativeThread.joinable())
        m_nativeThread.join();
     m_currentRunnableClosure = nullptr;
}

void cAdvThread::stop()
{
    if(m_currentRunnableClosure)
        m_currentRunnableClosure(int(eRUN_MODES::STOP), 0);//StopRunnable

    m_isThreadWork = false;
    m_conditionVariable.notify_one();
}

void cAdvThread::stopWithoutDeleteExtraThread()
{
    if(m_currentRunnableClosure)
        m_currentRunnableClosure(int(eRUN_MODES::STOP_WITHOUT_EXTRA_THREAD_DELETING), 0);//StopRunnable

    m_isThreadWork = false;
    m_conditionVariable.notify_one();
}


QString cAdvThread::getWho()
{
    QString who;
    if(m_currentRunnableClosure)
        who = m_currentRunnableClosure(int(eRUN_MODES::WHO), 0);
    else
        who = QString("empty");

    return who;
}

int cAdvThread::getRunType()
{
    int runType;
    if(m_currentRunnableClosure)
        runType = m_currentRunnableClosure(int(eRUN_MODES::RUN_TYPE), 0).toInt();
    else
        runType = 0;

    return runType;
}

void cAdvThread::setCoreMask(int mask)
{
    m_affinityData.m_coreMask = mask;
    m_affinityData.m_coreChanged = 1;
    setAffinity();
}


void cAdvThread::threadMainFunction()
{
    getCurrentHandle();
    setAffinity();
    while (m_isThreadWork)
    {
        std::unique_lock<std::mutex> locker(m_threadMutex);
        // wait notification and check that it does not a false awakening
        // Thread must awake if list is not empty or it was poweroff
        m_conditionVariable.wait(locker, [&](){ return !m_taskArray.empty() || !m_isThreadWork || m_affinityData.m_coreChanged;});

        while(!m_taskArray.empty())
        {
            if(!m_isThreadWork)//if stop thread of pool
            {
                while(!m_taskArray.empty())//stop all tasks
                {
                    runnable_closure runClosure = m_taskArray.front();//get task from array
                    m_taskArray.pop();
                    locker.unlock();//unlock before task call
                    runClosure(int(eRUN_MODES::STOP), 0);
                    locker.lock();
                }
                return;
            }

            m_currentRunnableClosure = m_taskArray.front();//get task from array
            m_taskArray.pop();

            locker.unlock();//unlock before task call
            try
            {
                if(m_threadType == eThreadType::THREAD_SHARED && getRunType() == eRunnableType::SHORT_TASK)
                {
                    QString stringTimerResult = m_currentRunnableClosure(int(eRUN_MODES::IS_TIMER_OVER), 0);
                    if(stringTimerResult == QString("1"))
                    {
                        m_lastTimeOfTaskLaunch = std::chrono::high_resolution_clock::now();
                        m_isReadyForSend_WarningAboutShortTaskFreeze = true;
                        m_currentRunnableClosure(int(eRUN_MODES::RUN), 0);
                        m_isReadyForSend_WarningAboutShortTaskFreeze = false;
                    }
                    else //task hold over
                        appendRunnableTask(m_currentRunnableClosure, eRunnableType::SHORT_TASK);
                }
                else
                {
                    m_lastTimeOfTaskLaunch = std::chrono::high_resolution_clock::now();
                    m_isReadyForSend_WarningAboutShortTaskFreeze = true;
                    m_currentRunnableClosure(int(eRUN_MODES::RUN), 0);
                    m_isReadyForSend_WarningAboutShortTaskFreeze = false;
                }

            }
            catch(ExceptionHoldOverTask action)
            {
                if(m_threadType == eThreadType::THREAD_SHARED && getRunType() == eRunnableType::SHORT_TASK)
                {
                    int counterOfExecution = m_currentRunnableClosure(int(eRUN_MODES::GET_COUNTER), 0).toInt();
                    if(counterOfExecution>0)
                    {
                        cAdvThreadPool::getInstance().getEmitter()->addWarningToShell(QString("Task was hold over - counter = %1 (%2)").arg(counterOfExecution)
                                                                                      .arg(QString::fromStdString(action.what())),
                                                                                      eLogWarning::MESSAGE);
                        m_currentRunnableClosure(int(eRUN_MODES::DECREASE_COUNTER), 0);
                        m_currentRunnableClosure(int(eRUN_MODES::START_TIMER_FOR_INTERVAL), action.getInterval());
                        appendRunnableTask(m_currentRunnableClosure, eRunnableType::SHORT_TASK);
                    }
                    else
                    {
                        cAdvThreadPool::getInstance().getEmitter()->addWarningToShell(QString("Task will not processing - counter over (%2)").arg(counterOfExecution)
                                                                                      .arg(QString::fromStdString(action.what())),
                                                                                      eLogWarning::WARNING);
                    }
                }

            }
            catch(std::exception& e)
            {
                QString temp = QString("%1 - CAdvThread::threadMainFunction - ").arg(e.what());
                temp += getWho();
                std::cout<<temp.toStdString()<<std::endl;
                cAdvThreadPool::getInstance().getEmitter()->addWarningToShell(temp, eLogWarning::WARNING);
            }
            catch(...)
            {               
                QString temp = QString("Undefined exception - CAdvThread::threadMainFunction - ");
                temp += getWho();
                std::cout<<temp.toStdString()<<std::endl;
                cAdvThreadPool::getInstance().getEmitter()->addWarningToShell(temp, eLogWarning::WARNING);
            }

            locker.lock(); // lock before m_TaskArray.empty()
            m_currentRunnableClosure = nullptr;

            if(m_threadType == eThreadType::THREAD_NOT_SHARED)
                cAdvThreadPool::getInstance().getEmitter()->sendSignal_DeleteLongTask(getThreadNumber());
            else if(m_threadType == eThreadType::THREAD_NOT_SHARED_EXTRA)
                cAdvThreadPool::getInstance().getEmitter()->sendSignal_DeleteExtraLongTask(getThreadNumber());
        }
    }
}
