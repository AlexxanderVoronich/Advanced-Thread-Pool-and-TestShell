// FILE AdvPoolEmitter.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef CPOOLEMITTER_H
#define CPOOLEMITTER_H

#include "AdvThread.h"
#include "ServiceStructures.h"
#include <QObject>

class CAdvPoolEmitter: public QObject
{
    Q_OBJECT

public://methods
    CAdvPoolEmitter(){}
    ~CAdvPoolEmitter(){}

    void sendSignal_PoolStart()
    {
        emit signal_PoolState(1);
    }
    void sendSignal_PoolStop()
    {
        emit signal_PoolState(0);
    }
    void sendSignal_NewThread(int id, CAdvThread::eThreadType type)
    {
        emit signal_NewThread(id, (int)type);
    }
    void sendSignal_Who_forUnsharedThread(int id, int run_type, QString who)
    {
        emit signal_UnsharedThread_AddLongTask(id, run_type, who);
    }
    void sendSignal_longTaskQuantity(int quantity)
    {
        emit signal_LongTaskQuantity(quantity);
    }
    void sendSignal_MeanCountTasks(int id, QString count)
    {
        emit signal_SharedThread_MeanCountTasks(id, count);
    }
    void sendSignal_DeleteLongTask(int id)
    {
        emit signal_UnsharedThread_DeleteLongTask(id);
    }
    void sendSignal_DeleteExtraLongTask(int id)
    {
        emit signal_UnsharedThread_DeleteExtraLongTask(id);
    }

    void sendSignal_poolNotWork()
    {
        //emit signal_DeleteTopTaskFromThread(id);
    }
    void sendSignal_NoThread()
    {
        //emit signal_DeleteTopTaskFromThread(id);
        addWarningToShell(QString("Thread not find"), eLogWarning::warning);
    }

    void sendSignal_AddRepeatTask(int task_id, int time, QString who, int repeatTaskQuantity)
    {
        emit signal_AddRepeatTask(task_id, time, who, repeatTaskQuantity);
    }
    void sendSignal_DeleteRepeatTask(int task_id)
    {
        emit signal_DeleteRepeatTask(task_id);
    }
    void sendSignal_EditRepeatTaskTime(int task_id, int time)
    {
        emit signal_EditRepeatTaskTime(task_id, time);
    }

    void sendSignals_AffinityMode(eAffinityMode affinityMode)
    {
        emit signal_AffinityMode(affinityMode);
    }

    void sendSignals_Info(int unshared, int shared, int coreQuantity, eAffinityMode affinityMode)
    {
        emit signal_QuantityUnsharedThreads(unshared);
        emit signal_QuantitySharedThreads(shared);
        emit signal_QuantityCores(coreQuantity);
        emit signal_AffinityMode(affinityMode);
    }

    void sendSignal_AffinityMask(int thread_num, int thread_type, int thread_mask)
    {
        emit signal_AffinityMask(thread_num, thread_type, thread_mask);
    }

    //add new warning in Warning_Table
    void addWarningToShell(QString message, eLogWarning value)
    {
        emit signal_AddWarning(message, value);
    }

signals:
    void signal_PoolState(int);
    void signal_NewThread(int, int);
    void signal_UnsharedThread_AddLongTask(int, int, QString);
    void signal_AddExtraUnsharedThread(int, int, QString);
    void signal_UnsharedThread_DeleteLongTask(int);
    void signal_UnsharedThread_DeleteExtraLongTask(int);
    void signal_LongTaskQuantity(int);
    void signal_SharedThread_MeanCountTasks(int, QString);
    void signal_AddRepeatTask(int, int, QString, int);
    void signal_DeleteRepeatTask(int);
    void signal_EditRepeatTaskTime(int, int);
    void signal_QuantitySharedThreads(int);
    void signal_QuantityUnsharedThreads(int);
    void signal_QuantityCores(int);
    void signal_AffinityMode(eAffinityMode);
    void signal_AffinityMask(int, int, int);
    //void signal_setMainThreadAffinity(int);
    void signal_AddWarning(QString text, eLogWarning);
};

#endif // CPOOLEMITTER_H
