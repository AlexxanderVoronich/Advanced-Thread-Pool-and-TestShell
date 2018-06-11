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
#ifdef Q_OS_WIN
#include <windows.h>
#endif

class CAdvThread
{
public://enums
    enum class eThreadType{THREAD_NO_TYPE = 0, THREAD_SHARED, THREAD_NOT_SHARED, THREAD_NOT_SHARED_EXTRA};
    enum class eRUN_MODES {RUN = 1, STOP, STOP_WITHOUT_EXTRA_THREAD_DELETING, WHO, RUN_TYPE, GET_COUNTER, DECREASE_COUNTER, START_TIMER_FOR_INTERVAL, IS_TIMER_OVER};

private://fields
    std::condition_variable conditionVariable;
    std::queue<runnable_closure> taskArray;
    runnable_closure currentRunnableClosure;

    std::mutex threadMutex;
    std::thread nativeThread;
    bool isThreadWork = false;
    int threadNumber = 0;
    eThreadType threadType = eThreadType::THREAD_NO_TYPE;
    int coreQuantity = 0;
    int averageTaskQuantity = 0;
    //std::chrono::system_clock::time_point lastTimeOfTaskQuantityAnalize;
    //std::chrono::system_clock::time_point lastTimeOfTaskLaunch;

    std::chrono::high_resolution_clock::time_point lastTimeOfTaskQuantityAnalize;
    std::chrono::high_resolution_clock::time_point lastTimeOfTaskLaunch;

    SAffinityData affinityData;

#ifdef Q_OS_LINUX
    pthread_t threadHandle = 0;
    cpu_set_t cpuset;
#else
    HANDLE threadHandle = 0;
    unsigned int thread_id = 0;
#endif

public://methods
    CAdvThread(int a_iThreadNumber, int  a_iCoreQuantity);
    ~CAdvThread();

    bool appendRunnableTask(runnable_closure run, int run_type);
    size_t getTaskCount();
    size_t getAllTaskCount();
    //std::chrono::system_clock::time_point getLastTimeOfTaskLaunch()const;
    std::chrono::time_point<std::chrono::high_resolution_clock> getLastTimeOfTaskLaunch() const;

    bool isReadyForSend_WarningAboutShortTaskFreeze = false;

    bool isEmpty();
    void wait();
    void stop();
    void stopWithoutDeleteExtraThread();
    bool isStoped(){ return !isThreadWork;}
    int getThreadNumber(){return threadNumber;}
    QString getWho();
    int getRunType();
    void setThreadType(eThreadType type){ threadType = type;}
    eThreadType getThreadType(){ return threadType;}
    void setCoreMask(int mask);

private://methods
    void threadMainFunction();
    void getCurrentHandle();
    void setAffinity();
};

typedef std::shared_ptr<CAdvThread> adv_thread_ptr;

#endif // ADVTHREAD_H
