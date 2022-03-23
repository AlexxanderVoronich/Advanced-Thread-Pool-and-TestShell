// FILE AdvThread.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef ADVTHREAD_H
#define ADVTHREAD_H

#include "Runnable.h"
#include "AdvThread.h"
#include "ServiceStructures.h"
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <chrono>
#include <QObject>
#include "DllHeader.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

class IMPORT_EXPORT cAdvThread
{
public://enums
    enum class eThreadType{THREAD_NO_TYPE = 0, THREAD_SHARED, THREAD_NOT_SHARED, THREAD_NOT_SHARED_EXTRA};
    enum class eRUN_MODES {RUN = 1, STOP, STOP_WITHOUT_EXTRA_THREAD_DELETING, WHO, RUN_TYPE, GET_COUNTER, DECREASE_COUNTER, START_TIMER_FOR_INTERVAL, IS_TIMER_OVER};

private://fields
    std::condition_variable m_conditionVariable;
    std::queue<runnable_closure> m_taskArray;
    runnable_closure m_currentRunnableClosure;

    std::mutex m_threadMutex;
    std::thread m_nativeThread;
    bool m_isThreadWork = false;
    int m_threadNumber = 0;
    eThreadType m_threadType = eThreadType::THREAD_NO_TYPE;
    int m_coreQuantity = 0;
    int m_averageTaskQuantity = 0;
    //std::chrono::system_clock::time_point lastTimeOfTaskQuantityAnalize;
    //std::chrono::system_clock::time_point lastTimeOfTaskLaunch;

    std::chrono::high_resolution_clock::time_point m_lastTimeOfTaskQuantityAnalize;
    std::chrono::high_resolution_clock::time_point m_lastTimeOfTaskLaunch;

    SAffinityData m_affinityData;

#ifdef Q_OS_LINUX
    pthread_t threadHandle = 0;
    cpu_set_t cpuset;
#else
    HANDLE threadHandle = 0;
    unsigned int thread_id = 0;
#endif

public://methods
    cAdvThread(int _threadNumber, int _coreQuantity);
    ~cAdvThread();

    bool appendRunnableTask(runnable_closure _run, int _runType);
    size_t getTaskCount();
    size_t getAllTaskCount();
    //std::chrono::system_clock::time_point getLastTimeOfTaskLaunch()const;
    std::chrono::time_point<std::chrono::high_resolution_clock> getLastTimeOfTaskLaunch() const;

    bool m_isReadyForSend_WarningAboutShortTaskFreeze = false;

    bool isEmpty();
    void wait();
    void stop();
    void stopWithoutDeleteExtraThread();
    bool isStoped(){ return !m_isThreadWork;}
    int getThreadNumber(){return m_threadNumber;}
    QString getWho();
    int getRunType();
    void setThreadType(eThreadType type){ m_threadType = type;}
    eThreadType getThreadType(){ return m_threadType;}
    void setCoreMask(int mask);

private://methods
    void threadMainFunction();
    void getCurrentHandle();
    void setAffinity();
};

typedef std::shared_ptr<cAdvThread> adv_thread_ptr;

#endif // ADVTHREAD_H
