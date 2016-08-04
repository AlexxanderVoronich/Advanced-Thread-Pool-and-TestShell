// FILE AdvThreadPool.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "AdvThreadPool.h"
#include <qthread.h>
#include <QtDebug>
#include <fstream>
#include "iostream"

//CThreadPoolSettings CAdvThreadPool::poolSettings = CThreadPoolSettings();

CAdvThreadPool::CAdvThreadPool()
{
    qRegisterMetaType<eLogWarning>();
    poolSettings.isPoolStarted = false;
    emitter = new CAdvPoolEmitter();
    /*
#ifdef WINDOWS_OS
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    poolSettings.m_iCoreQuantity = sysinfo.dwNumberOfProcessors;
#else //LINUX/SOLARIS
    sysconf(_SC_NPROCESSORS_ONLN);
    poolSettings.m_iCoreQuantity = _SC_NPROCESSORS_ONLN;
#endif
    */

    poolSettings.coreQuantity = std::thread::hardware_concurrency();
    if(poolSettings.coreQuantity>32)
        poolSettings.coreQuantity = 4;
}

bool CAdvThreadPool::createThreadPool(int unsharedThreadsQuantity, int sharedThreadsQuantity)
{
    try
    {
        if(!poolSettings.readArchive() &&
                unsharedThreadsQuantity != -1)
        {
            poolSettings.setQuantityUnsharedThreads(unsharedThreadsQuantity);
            poolSettings.setQuantitySharedThreads(sharedThreadsQuantity);
        }
        else if(!poolSettings.isValid()&&
                unsharedThreadsQuantity != -1)
        {
            poolSettings.setQuantityUnsharedThreads(unsharedThreadsQuantity);
            poolSettings.setQuantitySharedThreads(sharedThreadsQuantity);
        }

        if(poolSettings.masks.empty())
        {
            poolSettings.masks.resize(poolSettings.getQuantityUnsharedThreads() +
                    poolSettings.getQuantitySharedThreads());
            for(int i=0; i<poolSettings.getQuantityUnsharedThreads() +
                poolSettings.getQuantitySharedThreads(); i++)
            {
                poolSettings.masks[i] = INT_MAX;
            }
        }

        int i;
        for (i = 0; i<poolSettings.getQuantityUnsharedThreads(); i++)
        {
            auto affinityMode = poolSettings.getAffinityMode();
            adv_thread_ptr pThread(new CAdvThread(i+1, poolSettings.coreQuantity));
            pThread->setThreadType(CAdvThread::eThreadType::THREAD_NOT_SHARED);
            if(affinityMode == eAffinityMode::Yes_Affinity ||
                affinityMode == eAffinityMode::Yes_Affinity_Without_GUI_Edition)
                pThread->setCoreMask(poolSettings.masks[i]);
            threadsArray.push_back(pThread);
        }
        for (; i<poolSettings.getQuantityUnsharedThreads()+poolSettings.getQuantitySharedThreads(); i++)
        {
            auto affinityMode = poolSettings.getAffinityMode();
            adv_thread_ptr pThread(new CAdvThread(i+1, poolSettings.coreQuantity));
            pThread->setThreadType(CAdvThread::eThreadType::THREAD_SHARED);
            if(affinityMode == eAffinityMode::Yes_Affinity ||
                affinityMode == eAffinityMode::Yes_Affinity_Without_GUI_Edition)
                pThread->setCoreMask(poolSettings.masks[i]);
            threadsArray.push_back(pThread);
        }

        poolSettings.isPoolStarted = true;

        if(poolSettings.poolModes.threadPoolMode == eThreadPoolMode::REPEATED_TASK_MODE)
        {
            systemTymerRunnable = new CLongTask<CAdvThreadPool, int>(this,
                                                        &CAdvThreadPool::startPoolMainFunction,
                                                        &CAdvThreadPool::stopSystemTymerRunnable,
                                                        QString("System pool thread"));

            CAdvThreadPool::launchRunnableObject<int, CLongTask<CAdvThreadPool, int>>(systemTymerRunnable, true);
        }
    }
    catch(std::exception& e)
    {
        poolSettings.isPoolStarted = false;
        std::cout<<e.what();
    }

    return poolSettings.isPoolStarted;
}

adv_thread_ptr CAdvThreadPool::createAdditionalNotSharedThread(int& id)
{
    std::unique_lock<std::mutex> locker(additionalThreadsMutex);
    int extraThreadsQuantity = 1000 + additionalThreadsArray.size();
    adv_thread_ptr pThread(new CAdvThread(extraThreadsQuantity+1, poolSettings.coreQuantity));
    pThread->setThreadType(CAdvThread::eThreadType::THREAD_NOT_SHARED_EXTRA);
    additionalThreadsArray.push_back(pThread);
    QString who = pThread->getWho();
    id = pThread->getThreadNumber();

    emitter->sendSignal_NewThread(id, pThread->getThreadType());
    return pThread;
}
/*
int CAdvThreadPool::setMainThreadAffinity(int mask)
{
#ifdef Q_OS_LINUX
    cpu_set_t cpuset;
    pthread_t thread;
    thread = pthread_self();
    CPU_ZERO(&cpuset);
#else
    HANDLE thread = GetCurrentThread();
#endif

#ifdef Q_OS_LINUX
    int j;
    for (j = 0; j < poolSettings.coreQuantity; j++)
        if(mask & (1<<j))
            CPU_SET(j, &cpuset);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
#else
     DWORD_PTR res = SetThreadAffinityMask(thread, mask);
     if (0 == res)
     {
         // failure
     }
#endif
}*/

int CAdvThreadPool::startPoolMainFunction()
{
    systemThreadRunnableSign = true;
    bool isAdditionalThreadsQuantityChanged = false;

    int timeout;
    infoTimer.start();
    while(systemThreadRunnableSign)
    {
        QThread::msleep(10);

        //check repeated tasks
        std::unique_lock<std::mutex> locker(repeatTaskMutex);
        for(auto it = repeatedTaskArray.begin(); it != repeatedTaskArray.end(); it++)
        {
            timeout = it->qt_timer.elapsed();

            if(timeout >= it->timePeriod)
            {
                int threadId = -1;
                QString runType = it->run(int(CAdvThread::eRUN_MODES::RUN_TYPE), 0);
                adv_thread_ptr pThread;

                pThread = getFreeThread(threadId, runType.toInt());
                if(pThread == nullptr)
                {
                    emitter->sendSignal_NoThread();
                    continue;
                }
                pThread->appendRunnableTask(it->run, runType.toInt());
                it->qt_timer.start();
            }
        }


        //notification about suspicious behavior
        auto currentTime = std::chrono::high_resolution_clock::now();

        for (auto &it : threadsArray)
        {
            CAdvThread::eThreadType l_iThreadType = it->getThreadType();
            if(l_iThreadType == CAdvThread::eThreadType::THREAD_SHARED) //SHORT_TASK || REPEAT_TASK
            {
                if (!(it->isEmpty()))
                {
                    auto lastTimeOfTaskLaunch = it->getLastTimeOfTaskLaunch();
                    auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTimeOfTaskLaunch).count();
                    if((it->isReadyForSend_WarningAboutShortTaskFreeze) && deltaTime > 5000)//very suspicious
                    {
                        getEmitter()->addWarningToShell(QString("Maybe task '%1' was frozen (very suspicious)").arg(it->getWho()), eLogWarning::warning);
                        it->isReadyForSend_WarningAboutShortTaskFreeze = false;
                    }
                }
            }
        }

        //delete extra long threads which was stoped
        additionalThreadsMutex.lock();
        isAdditionalThreadsQuantityChanged = false;
        additionalThreadsArray.remove_if(
        [&isAdditionalThreadsQuantityChanged](adv_thread_ptr thr)
        {
            if(thr->isStoped())
            {
                qDebug() << "Remove thread from additionalThreadsArray";
                isAdditionalThreadsQuantityChanged = true;
                return true;
            }
            else
                return false;
        });
        additionalThreadsMutex.unlock();
        if(isAdditionalThreadsQuantityChanged)
        {
            int longTaskCounter = CAdvThreadPool::getInstance().getLongTaskQuantity();
            emitter->sendSignal_longTaskQuantity(longTaskCounter);
        }
    }
    return 0;
}

void CAdvThreadPool::stopSystemTymerRunnable()
{
    systemThreadRunnableSign = false;
}

CAdvThreadPool::~CAdvThreadPool()
{
    delete emitter;
}

void CAdvThreadPool::startThreadPool(int unsharedThreadsQuantity, int sharedThreadsQuantity, CPoolModes poolModes, QString file)
{
    if(unsharedThreadsQuantity != -1)
    {
        getSettings().filePath = file;
        getSettings().poolModes = poolModes;
    }

    gens<10> g;
    if(!getSettings().isPoolStarted)
    {
        getSettings().isPoolStarted = CAdvThreadPool::getInstance().createThreadPool(unsharedThreadsQuantity, sharedThreadsQuantity);
        CAdvThreadPool::getInstance().getEmitter()->sendSignal_PoolStart();
        CAdvThreadPool::getInstance().getEmitter()->addWarningToShell(QString("Pool has started"), eLogWarning::message);
    }
}

void CAdvThreadPool::stopThreadPool()
{
    CAdvThreadPool::getInstance().stopAll();
    CAdvThreadPool::getInstance().waitAll();
    getSettings().isPoolStarted = false;
    CAdvThreadPool::getInstance().getEmitter()->sendSignal_PoolStop();
    CAdvThreadPool::getInstance().getEmitter()->addWarningToShell(QString("Pool has stoped"), eLogWarning::message);
}

CAdvThreadPool &CAdvThreadPool::getInstance()
{
    static CAdvThreadPool pool;
    return pool;
}

CThreadPoolSettings &CAdvThreadPool::getSettings()
{
    return getInstance().poolSettings;
}

adv_thread_ptr CAdvThreadPool::getFreeThread(int &thread_id, int runType)
{
    adv_thread_ptr pFoundedThread = nullptr;
    size_t minTasks = UINT32_MAX;
    for (auto &pThread : threadsArray)
    {
        CAdvThread::eThreadType threadType = pThread->getThreadType();
        if(threadType == CAdvThread::eThreadType::THREAD_NOT_SHARED &&
               (runType == int(eRunnableType::SHORT_TASK) || runType == int(eRunnableType::REPEAT_TASK)))
            continue;
        else if(threadType == CAdvThread::eThreadType::THREAD_SHARED &&
                (runType == int(eRunnableType::LONG_TASK)))
            continue;

        if (pThread->isEmpty())
        {
            thread_id = pThread->getThreadNumber();
            return pThread;
        }

        if(runType == int(eRunnableType::LONG_TASK))
            continue;
        else if (minTasks > pThread->getTaskCount())
        {
            minTasks = pThread->getTaskCount();
            pFoundedThread = pThread;
            thread_id = pThread->getThreadNumber();
        }
    }
    return pFoundedThread;
}

adv_thread_ptr CAdvThreadPool::createSystemPoolThread()
{
    if(systemPoolThread != nullptr)
        return nullptr;

    adv_thread_ptr pThread(new CAdvThread(0, poolSettings.coreQuantity));
    systemPoolThread = pThread;
    systemPoolThread->setThreadType(CAdvThread::eThreadType::THREAD_NOT_SHARED);

    return systemPoolThread;
}

int CAdvThreadPool::getLongTaskQuantity()
{
    int longTaskCounter = 0;
    for(adv_thread_ptr &threadObject : threadsArray)
    {
        CAdvThread::eThreadType threadType = threadObject->getThreadType();
        if(threadType == CAdvThread::eThreadType::THREAD_NOT_SHARED &&
             !(threadObject->isEmpty()))
            longTaskCounter++;
    }

    std::unique_lock<std::mutex> locker(additionalThreadsMutex);
    for(adv_thread_ptr &threadObject : additionalThreadsArray)
    {
        CAdvThread::eThreadType threadType = threadObject->getThreadType();
        if(threadType == CAdvThread::eThreadType::THREAD_NOT_SHARED_EXTRA &&
             !(threadObject->isEmpty()))
            longTaskCounter++;
    }
    return longTaskCounter;
}

void CAdvThreadPool::stopAll()
{
    if(systemPoolThread != nullptr)
    {
        systemPoolThread->stop();
        systemPoolThread->wait();
        systemPoolThread.reset();
    }

    for(adv_thread_ptr &it : threadsArray)
    {
        it->stop();
    }

    std::unique_lock<std::mutex> locker(additionalThreadsMutex);
    for(adv_thread_ptr &it : additionalThreadsArray)
    {
        it->stopWithoutDeleteExtraThread();
    }
}

void CAdvThreadPool::waitAll()
{
    for(adv_thread_ptr &it : threadsArray)
    {
        it->wait();
    }

    threadsArray.clear();

    std::unique_lock<std::mutex> locker(additionalThreadsMutex);
    for(adv_thread_ptr &it : additionalThreadsArray)
    {
        it->wait();
    }

    additionalThreadsArray.clear();
}

void CAdvThreadPool::deleteExtraThread(int id, bool sign)
{
    /*if(!sign)
        std::async<>(std::launch::async,&CAdvThreadPool::deleteExtraThread, this, id, true);
    else*/
    {
        {
            std::unique_lock<std::mutex> locker(additionalThreadsMutex);

            for(adv_thread_ptr thr : additionalThreadsArray)
            {
                if(thr->getThreadNumber() == id)
                {
                    thr->stop();
                }
            }

            /*additionalThreadsArray.remove_if(
            [id](adv_thread_ptr thr)
            {
                if(thr->getThreadID() == id)
                {
                    thr->stop();
                    return true;
                }
                else
                    return false;
            });*/
        }

        /*std::unique_lock<std::mutex> locker(additionalThreadsMutex);
        int longTaskCounter = CAdvThreadPool::getInstance().getLongTaskQuantity();
        //--longTaskCounter;
        emitter->sendSignal_longTaskQuantity(longTaskCounter);*/
    }
}

void CAdvThreadPool::requestState()
{
    emitter->sendSignals_Info(poolSettings.getQuantityUnsharedThreads(),
                                 poolSettings.getQuantitySharedThreads(),
                                 poolSettings.coreQuantity,
                                 poolSettings.getAffinityMode());

    if(systemPoolThread != nullptr)
    {
        int id = systemPoolThread->getThreadNumber();
        QString who = systemPoolThread->getWho();
        int runType =  systemPoolThread->getRunType();
        emitter->sendSignal_NewThread(id, systemPoolThread->getThreadType());
        emitter->sendSignal_Who_forUnsharedThread(id, runType, who);
        int longTaskCounter = CAdvThreadPool::getInstance().getLongTaskQuantity();
        emitter->sendSignal_longTaskQuantity(longTaskCounter);
    }

    for (auto it = threadsArray.begin(); it != threadsArray.end(); it++)
    {
        int id = (*it)->getThreadNumber();
        QString who = (*it)->getWho();
        int runType =  (*it)->getRunType();
        emitter->sendSignal_NewThread(id, (*it)->getThreadType());

        if((*it)->getThreadType() == CAdvThread::eThreadType::THREAD_NOT_SHARED)
        {
            emitter->sendSignal_Who_forUnsharedThread(id, runType, who);
            int longTaskCounter = CAdvThreadPool::getInstance().getLongTaskQuantity();
            emitter->sendSignal_longTaskQuantity(longTaskCounter);
        }
        else
            emitter->sendSignal_MeanCountTasks(id,  QString::number((*it)->getAllTaskCount()));
    }

    for(int i=1; i<=(poolSettings.getQuantityUnsharedThreads() + poolSettings.getQuantitySharedThreads()); i++)
    {
        if(i<=poolSettings.getQuantityUnsharedThreads())
            emitter->sendSignal_AffinityMask(i, int(CAdvThread::eThreadType::THREAD_NOT_SHARED), poolSettings.masks[i-1]);
        else
            emitter->sendSignal_AffinityMask(i, int(CAdvThread::eThreadType::THREAD_SHARED), poolSettings.masks[i-1]);
    }

    emitter->sendSignals_AffinityMode(poolSettings.getAffinityMode());

    if(poolSettings.isPoolStarted)
        emitter->sendSignal_PoolStart();
    else
        emitter->sendSignal_PoolStop();
}

void CAdvThreadPool::addRepeatedTask(runnable_closure task, int ID, int timePeriod, QString who)
{
    std::unique_lock<std::mutex> locker(repeatTaskMutex);
    CRepeatedTaskAdapter wrapper(task);
    wrapper.qt_timer.start();
    wrapper.ID = ID;
    wrapper.timePeriod = timePeriod;
    repeatedTaskArray.push_back(wrapper);

    emitter->sendSignal_AddRepeatTask(ID, timePeriod, who, repeatedTaskArray.size());
}

bool CAdvThreadPool::stopRepeatTask(int taskID)
{
    getInstance().stopRunnable_RepeatTask(taskID);
}

bool CAdvThreadPool::stopRunnable_RepeatTask(int taskID)
{
    std::unique_lock<std::mutex> locker(repeatTaskMutex);
    auto it = std::find_if(repeatedTaskArray.begin(), repeatedTaskArray.end(),
                           [taskID](CRepeatedTaskAdapter &wrap)->bool
                            {if(wrap.ID == taskID) return true;
                                else return false;});
    if(it == repeatedTaskArray.end())
    {
        return false;
    }

    //it->run(CAdvThread::eRUN_MODES::STOP);
    repeatedTaskArray.erase(it);
    emitter->sendSignal_DeleteRepeatTask(taskID);
    return true;
}

bool CAdvThreadPool::updateTimePeriodForRepeatTask(int taskID, int newTimePeriod)
{
    std::unique_lock<std::mutex> locker(repeatTaskMutex);
    auto it = std::find_if(repeatedTaskArray.begin(), repeatedTaskArray.end(),
                           [taskID](CRepeatedTaskAdapter &wrap)->bool
                            {if(wrap.ID == taskID) return true;
                                else return false;});
    if(it == repeatedTaskArray.end())
    {
        return false;
    }

    it->timePeriod = newTimePeriod;

    emitter->sendSignal_EditRepeatTaskTime(taskID, newTimePeriod);
    return true;
}

void CAdvThreadPool::saveSettings(int unsharedQ, int sharedQ, eAffinityMode affinityMode)
{
    bool deleteMaskArraySign = false;
    if(poolSettings.getQuantityUnsharedThreads() != unsharedQ ||
          poolSettings.getQuantitySharedThreads() != sharedQ)
        deleteMaskArraySign = true;

    poolSettings.setQuantityUnsharedThreads(unsharedQ);
    poolSettings.setQuantitySharedThreads(sharedQ);
    poolSettings.setAffinityMode(affinityMode);

    if(deleteMaskArraySign)
    {
        poolSettings.masks.clear();
        poolSettings.masks.resize(sharedQ + unsharedQ);
        for(int i=0; i<sharedQ + unsharedQ; i++)
        {
            poolSettings.masks[i] = INT_MAX;
        }
    }

    poolSettings.saveArchive();
}

//thread_id starts with 1
void CAdvThreadPool::changeAffinityMask(int threadID, int coreID, bool state)
{
    if(threadID == 0)//thread system thread
        return;

    if(threadID > poolSettings.getQuantityUnsharedThreads() + poolSettings.getQuantitySharedThreads())
        return;
    try
    {
        int mask = calculateCoreMask(poolSettings.masks[threadID-1], coreID, state);

        auto it = std::find_if(threadsArray.begin(), threadsArray.end(),
                   [threadID](adv_thread_ptr _thread)
                    {
                        if(_thread->getThreadNumber() == threadID)
                            return true;
                        else return false;
                    }
                );

        if(it != threadsArray.end())
        {
            (*it)->setCoreMask(mask);
            poolSettings.masks[threadID-1] = mask;
        }
        else
            return;
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        return;
    }
}

int CAdvThreadPool::calculateCoreMask(int mask, int core_num, bool value)
{
    if(value)
        mask |= (1<<(core_num-1));
    else
        mask &= ~(1<<(core_num-1));

    return mask;
}
