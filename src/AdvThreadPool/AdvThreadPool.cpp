// FILE AdvThreadPool.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "AdvThreadPool.h"
#include <qthread.h>
#include <QtDebug>
#include <fstream>
#include "iostream"

//CThreadPoolSettings cAdvThreadPool::poolSettings = cThreadPoolSettings();

cAdvThreadPool::cAdvThreadPool()
{
    qRegisterMetaType<eLogWarning>();
    m_poolSettings.m_isPoolStarted = false;
    m_emitter = new cAdvPoolEmitter();
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

    m_poolSettings.m_coreQuantity = std::thread::hardware_concurrency();
    if(m_poolSettings.m_coreQuantity>32)
        m_poolSettings.m_coreQuantity = 4;
}

bool cAdvThreadPool::createThreadPool(int unsharedThreadsQuantity, int sharedThreadsQuantity)
{
    try
    {
        if(!m_poolSettings.readArchive() &&
                unsharedThreadsQuantity != -1)
        {
            m_poolSettings.setQuantityUnsharedThreads(unsharedThreadsQuantity);
            m_poolSettings.setQuantitySharedThreads(sharedThreadsQuantity);
        }
        else if(!m_poolSettings.isValid()&&
                unsharedThreadsQuantity != -1)
        {
            m_poolSettings.setQuantityUnsharedThreads(unsharedThreadsQuantity);
            m_poolSettings.setQuantitySharedThreads(sharedThreadsQuantity);
        }

        if(m_poolSettings.m_masks.empty())
        {
            m_poolSettings.m_masks.resize(m_poolSettings.getQuantityUnsharedThreads() +
                    m_poolSettings.getQuantitySharedThreads());
            for(int i=0; i<m_poolSettings.getQuantityUnsharedThreads() +
                m_poolSettings.getQuantitySharedThreads(); i++)
            {
                m_poolSettings.m_masks[i] = INT_MAX;
            }
        }

        int i;
        for (i = 0; i<m_poolSettings.getQuantityUnsharedThreads(); i++)
        {
            auto affinityMode = m_poolSettings.getAffinityMode();
            adv_thread_ptr pThread(new cAdvThread(i+1, m_poolSettings.m_coreQuantity));
            pThread->setThreadType(cAdvThread::eThreadType::THREAD_NOT_SHARED);
            if(affinityMode == eAffinityMode::YES_AFFINITY ||
                affinityMode == eAffinityMode::Yes_Affinity_Without_GUI_Edition)
                pThread->setCoreMask(m_poolSettings.m_masks[i]);
            threadsArray.push_back(pThread);
        }
        for (; i<m_poolSettings.getQuantityUnsharedThreads()+m_poolSettings.getQuantitySharedThreads(); i++)
        {
            auto affinityMode = m_poolSettings.getAffinityMode();
            adv_thread_ptr pThread(new cAdvThread(i+1, m_poolSettings.m_coreQuantity));
            pThread->setThreadType(cAdvThread::eThreadType::THREAD_SHARED);
            if(affinityMode == eAffinityMode::YES_AFFINITY ||
                affinityMode == eAffinityMode::Yes_Affinity_Without_GUI_Edition)
                pThread->setCoreMask(m_poolSettings.m_masks[i]);
            threadsArray.push_back(pThread);
        }

        m_poolSettings.m_isPoolStarted = true;

        if(m_poolSettings.m_poolModes.m_threadPoolMode == eThreadPoolMode::REPEATED_TASK_MODE)
        {
            m_systemTymerRunnable = new cLongTask<cAdvThreadPool, int>(this,
                                                        &cAdvThreadPool::startPoolMainFunction,
                                                        &cAdvThreadPool::stopSystemTymerRunnable,
                                                        QString("System pool thread"));

            cAdvThreadPool::launchRunnableObject<int, cLongTask<cAdvThreadPool, int>>(m_systemTymerRunnable, true);
        }
    }
    catch(std::exception& e)
    {
        m_poolSettings.m_isPoolStarted = false;
        std::cout<<e.what();
    }

    return m_poolSettings.m_isPoolStarted;
}

adv_thread_ptr cAdvThreadPool::createAdditionalNotSharedThread(int& _id)
{
    std::unique_lock<std::mutex> locker(additionalThreadsMutex);
    int extraThreadsQuantity = 1000 + additionalThreadsArray.size();
    adv_thread_ptr pThread(new cAdvThread(extraThreadsQuantity+1, m_poolSettings.m_coreQuantity));
    pThread->setThreadType(cAdvThread::eThreadType::THREAD_NOT_SHARED_EXTRA);
    additionalThreadsArray.push_back(pThread);
    QString who = pThread->getWho();
    _id = pThread->getThreadNumber();

    m_emitter->sendSignal_NewThread(_id, pThread->getThreadType());
    return pThread;
}
/*
int cAdvThreadPool::setMainThreadAffinity(int _mask)
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
        if(_mask & (1<<j))
            CPU_SET(j, &cpuset);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
#else
     DWORD_PTR res = SetThreadAffinityMask(thread, _mask);
     if (0 == res)
     {
         // failure
     }
#endif
}*/

int cAdvThreadPool::startPoolMainFunction()
{
    m_systemThreadRunnableSign = true;
    bool isAdditionalThreadsQuantityChanged = false;

    int timeout;
    infoTimer.start();
    while(m_systemThreadRunnableSign)
    {
        QThread::msleep(10);

        //check repeated tasks
        std::unique_lock<std::mutex> locker(repeatTaskMutex);
        for(auto it = repeatedTaskArray.begin(); it != repeatedTaskArray.end(); it++)
        {
            timeout = it->m_qt_timer.elapsed();

            if(timeout >= it->m_timePeriod)
            {
                int threadId = -1;
                QString runType = it->m_run(int(cAdvThread::eRUN_MODES::RUN_TYPE), 0);
                adv_thread_ptr pThread;

                pThread = getFreeThread(threadId, runType.toInt());
                if(pThread == nullptr)
                {
                    m_emitter->sendSignal_NoThread();
                    continue;
                }
                pThread->appendRunnableTask(it->m_run, runType.toInt());
                it->m_qt_timer.start();
            }
        }


        //notification about suspicious behavior
        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();

        for (auto &it : threadsArray)
        {
            cAdvThread::eThreadType l_iThreadType = it->getThreadType();
            if(l_iThreadType == cAdvThread::eThreadType::THREAD_SHARED) //SHORT_TASK || REPEAT_TASK
            {
                if (!(it->isEmpty()))
                {
                    auto lastTimeOfTaskLaunch = it->getLastTimeOfTaskLaunch();
                    auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTimeOfTaskLaunch).count();
                    if((it->m_isReadyForSend_WarningAboutShortTaskFreeze) && deltaTime > 5000)//very suspicious
                    {
                        getEmitter()->addWarningToShell(QString("Maybe task '%1' was frozen (very suspicious)").arg(it->getWho()), eLogWarning::WARNING);
                        it->m_isReadyForSend_WarningAboutShortTaskFreeze = false;
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
            int longTaskCounter = cAdvThreadPool::getInstance().getLongTaskQuantity();
            m_emitter->sendSignal_longTaskQuantity(longTaskCounter);
        }
    }
    return 0;
}

void cAdvThreadPool::stopSystemTymerRunnable()
{
    m_systemThreadRunnableSign = false;
}

cAdvThreadPool::~cAdvThreadPool()
{
    delete m_emitter;
}

void cAdvThreadPool::startThreadPool(int unsharedThreadsQuantity, int sharedThreadsQuantity, cPoolModes poolModes, QString file)
{
    if(unsharedThreadsQuantity != -1)
    {
        getSettings().m_filePath = file;
        getSettings().m_poolModes = poolModes;
    }

    gens<10> g;
    if(!getSettings().m_isPoolStarted)
    {
        getSettings().m_isPoolStarted = cAdvThreadPool::getInstance().createThreadPool(unsharedThreadsQuantity, sharedThreadsQuantity);
        cAdvThreadPool::getInstance().getEmitter()->sendSignal_PoolStart();
        cAdvThreadPool::getInstance().getEmitter()->addWarningToShell(QString("Pool has started"), eLogWarning::MESSAGE);
    }
}

void cAdvThreadPool::stopThreadPool()
{
    cAdvThreadPool::getInstance().stopAll();
    cAdvThreadPool::getInstance().waitAll();
    getSettings().m_isPoolStarted = false;
    cAdvThreadPool::getInstance().getEmitter()->sendSignal_PoolStop();
    cAdvThreadPool::getInstance().getEmitter()->addWarningToShell(QString("Pool has stoped"), eLogWarning::MESSAGE);
}

cAdvThreadPool &cAdvThreadPool::getInstance()
{
    static cAdvThreadPool pool;
    return pool;
}

cThreadPoolSettings &cAdvThreadPool::getSettings()
{
    return getInstance().m_poolSettings;
}

adv_thread_ptr cAdvThreadPool::getFreeThread(int &_thread_id, int _runType)
{
    adv_thread_ptr pFoundedThread = nullptr;
    size_t minTasks = UINT32_MAX;
    for (auto &pThread : threadsArray)
    {
        cAdvThread::eThreadType threadType = pThread->getThreadType();
        if(threadType == cAdvThread::eThreadType::THREAD_NOT_SHARED &&
               (_runType == int(eRunnableType::SHORT_TASK) || _runType == int(eRunnableType::REPEAT_TASK)))
            continue;
        else if(threadType == cAdvThread::eThreadType::THREAD_SHARED &&
                (_runType == int(eRunnableType::LONG_TASK)))
            continue;

        if (pThread->isEmpty())
        {
            _thread_id = pThread->getThreadNumber();
            return pThread;
        }

        if(_runType == int(eRunnableType::LONG_TASK))
            continue;
        else if (minTasks > pThread->getTaskCount())
        {
            minTasks = pThread->getTaskCount();
            pFoundedThread = pThread;
            _thread_id = pThread->getThreadNumber();
        }
    }
    return pFoundedThread;
}

adv_thread_ptr cAdvThreadPool::createSystemPoolThread()
{
    if(systemPoolThread != nullptr)
        return nullptr;

    adv_thread_ptr pThread(new cAdvThread(0, m_poolSettings.m_coreQuantity));
    systemPoolThread = pThread;
    systemPoolThread->setThreadType(cAdvThread::eThreadType::THREAD_NOT_SHARED);

    return systemPoolThread;
}

int cAdvThreadPool::getLongTaskQuantity()
{
    int longTaskCounter = 0;
    for(adv_thread_ptr &threadObject : threadsArray)
    {
        cAdvThread::eThreadType threadType = threadObject->getThreadType();
        if(threadType == cAdvThread::eThreadType::THREAD_NOT_SHARED &&
             !(threadObject->isEmpty()))
            longTaskCounter++;
    }

    std::unique_lock<std::mutex> locker(additionalThreadsMutex);
    for(adv_thread_ptr &threadObject : additionalThreadsArray)
    {
        cAdvThread::eThreadType threadType = threadObject->getThreadType();
        if(threadType == cAdvThread::eThreadType::THREAD_NOT_SHARED_EXTRA &&
             !(threadObject->isEmpty()))
            longTaskCounter++;
    }
    return longTaskCounter;
}

void cAdvThreadPool::stopAll()
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

void cAdvThreadPool::waitAll()
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

void cAdvThreadPool::deleteExtraThread(int _id, bool _sign)
{
    /*if(!sign)
        std::async<>(std::launch::async,&cAdvThreadPool::deleteExtraThread, this, id, true);
    else*/
    {
        {
            std::unique_lock<std::mutex> locker(additionalThreadsMutex);

            for(adv_thread_ptr thr : additionalThreadsArray)
            {
                if(thr->getThreadNumber() == _id)
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
        int longTaskCounter = cAdvThreadPool::getInstance().getLongTaskQuantity();
        //--longTaskCounter;
        emitter->sendSignal_longTaskQuantity(longTaskCounter);*/
    }
}

void cAdvThreadPool::requestState()
{
    m_emitter->sendSignals_Info(m_poolSettings.getQuantityUnsharedThreads(),
                                 m_poolSettings.getQuantitySharedThreads(),
                                 m_poolSettings.m_coreQuantity,
                                 m_poolSettings.getAffinityMode());

    if(systemPoolThread != nullptr)
    {
        int id = systemPoolThread->getThreadNumber();
        QString who = systemPoolThread->getWho();
        int runType =  systemPoolThread->getRunType();
        m_emitter->sendSignal_NewThread(id, systemPoolThread->getThreadType());
        m_emitter->sendSignal_Who_forUnsharedThread(id, runType, who);
        int longTaskCounter = cAdvThreadPool::getInstance().getLongTaskQuantity();
        m_emitter->sendSignal_longTaskQuantity(longTaskCounter);
    }

    for (auto it = threadsArray.begin(); it != threadsArray.end(); it++)
    {
        int id = (*it)->getThreadNumber();
        QString who = (*it)->getWho();
        int runType =  (*it)->getRunType();
        m_emitter->sendSignal_NewThread(id, (*it)->getThreadType());

        if((*it)->getThreadType() == cAdvThread::eThreadType::THREAD_NOT_SHARED)
        {
            m_emitter->sendSignal_Who_forUnsharedThread(id, runType, who);
            int longTaskCounter = cAdvThreadPool::getInstance().getLongTaskQuantity();
            m_emitter->sendSignal_longTaskQuantity(longTaskCounter);
        }
        else
            m_emitter->sendSignal_MeanCountTasks(id,  QString::number((*it)->getAllTaskCount()));
    }

    for(int i=1; i<=(m_poolSettings.getQuantityUnsharedThreads() + m_poolSettings.getQuantitySharedThreads()); i++)
    {
        if(i<=m_poolSettings.getQuantityUnsharedThreads())
            m_emitter->sendSignal_AffinityMask(i, int(cAdvThread::eThreadType::THREAD_NOT_SHARED), m_poolSettings.m_masks[i-1]);
        else
            m_emitter->sendSignal_AffinityMask(i, int(cAdvThread::eThreadType::THREAD_SHARED), m_poolSettings.m_masks[i-1]);
    }

    m_emitter->sendSignals_AffinityMode(m_poolSettings.getAffinityMode());

    if(m_poolSettings.m_isPoolStarted)
        m_emitter->sendSignal_PoolStart();
    else
        m_emitter->sendSignal_PoolStop();
}

void cAdvThreadPool::addRepeatedTask(runnable_closure _task, int _id, int _timePeriod, QString _who)
{
    std::unique_lock<std::mutex> locker(repeatTaskMutex);
    cRepeatedTaskAdapter wrapper(_task);
    wrapper.m_qt_timer.start();
    wrapper.m_id = _id;
    wrapper.m_timePeriod = _timePeriod;
    repeatedTaskArray.push_back(wrapper);

    m_emitter->sendSignal_AddRepeatTask(_id, _timePeriod, _who, repeatedTaskArray.size());
}

bool cAdvThreadPool::stopRepeatTask(int taskID)
{
    return getInstance().stopRunnable_RepeatTask(taskID);
}

bool cAdvThreadPool::stopRunnable_RepeatTask(int _taskID)
{
    std::unique_lock<std::mutex> locker(repeatTaskMutex);
    auto it = std::find_if(repeatedTaskArray.begin(), repeatedTaskArray.end(),
                           [_taskID](cRepeatedTaskAdapter &wrap)->bool
                            {if(wrap.m_id == _taskID) return true;
                                else return false;});
    if(it == repeatedTaskArray.end())
    {
        return false;
    }

    //it->run(CAdvThread::eRUN_MODES::STOP);
    repeatedTaskArray.erase(it);
    m_emitter->sendSignal_DeleteRepeatTask(_taskID);
    return true;
}

bool cAdvThreadPool::updateTimePeriodForRepeatTask(int _taskID, int _newTimePeriod)
{
    std::unique_lock<std::mutex> locker(repeatTaskMutex);
    auto it = std::find_if(repeatedTaskArray.begin(), repeatedTaskArray.end(),
                           [_taskID](cRepeatedTaskAdapter &wrap)->bool
                            {if(wrap.m_id == _taskID) return true;
                                else return false;});
    if(it == repeatedTaskArray.end())
    {
        return false;
    }

    it->m_timePeriod = _newTimePeriod;

    m_emitter->sendSignal_EditRepeatTaskTime(_taskID, _newTimePeriod);
    return true;
}

void cAdvThreadPool::saveSettings(int _unsharedQ, int _sharedQ, eAffinityMode _affinityMode)
{
    bool deleteMaskArraySign = false;
    if(m_poolSettings.getQuantityUnsharedThreads() != _unsharedQ ||
          m_poolSettings.getQuantitySharedThreads() != _sharedQ)
        deleteMaskArraySign = true;

    m_poolSettings.setQuantityUnsharedThreads(_unsharedQ);
    m_poolSettings.setQuantitySharedThreads(_sharedQ);
    m_poolSettings.setAffinityMode(_affinityMode);

    if(deleteMaskArraySign)
    {
        m_poolSettings.m_masks.clear();
        m_poolSettings.m_masks.resize(_sharedQ + _unsharedQ);
        for(int i=0; i<_sharedQ + _unsharedQ; i++)
        {
            m_poolSettings.m_masks[i] = INT_MAX;
        }
    }

    m_poolSettings.saveArchive();
}

//thread_id starts with 1
void cAdvThreadPool::changeAffinityMask(int _threadID, int _coreID, bool _state)
{
    if(_threadID == 0)//thread system thread
        return;

    if(_threadID > m_poolSettings.getQuantityUnsharedThreads() + m_poolSettings.getQuantitySharedThreads())
        return;
    try
    {
        int mask = calculateCoreMask(m_poolSettings.m_masks[_threadID-1], _coreID, _state);

        auto it = std::find_if(threadsArray.begin(), threadsArray.end(),
                   [_threadID](adv_thread_ptr _thread)
                    {
                        if(_thread->getThreadNumber() == _threadID)
                            return true;
                        else return false;
                    }
                );

        if(it != threadsArray.end())
        {
            (*it)->setCoreMask(mask);
            m_poolSettings.m_masks[_threadID-1] = mask;
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

int cAdvThreadPool::calculateCoreMask(int mask, int core_num, bool value)
{
    if(value)
        mask |= (1<<(core_num-1));
    else
        mask &= ~(1<<(core_num-1));

    return mask;
}
