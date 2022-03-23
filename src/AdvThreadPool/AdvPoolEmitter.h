// FILE AdvPoolEmitter.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef CPOOLEMITTER_H
#define CPOOLEMITTER_H

#include "AdvThread.h"
#include "ServiceStructures.h"
#include <QObject>
#include "DllHeader.h"

class IMPORT_EXPORT cAdvPoolEmitter: public QObject
{
    Q_OBJECT

public://methods
    cAdvPoolEmitter(){}
    ~cAdvPoolEmitter(){}

    void sendSignal_PoolStart()
    {
        emit signal_PoolState(1);
    }
    void sendSignal_PoolStop()
    {
        emit signal_PoolState(0);
    }
    void sendSignal_NewThread(int _id, cAdvThread::eThreadType _type)
    {
        emit signal_NewThread(_id, static_cast<int>(_type));
    }
    void sendSignal_Who_forUnsharedThread(int _id, int _run_type, QString _who)
    {
        emit signal_UnsharedThread_AddLongTask(_id, _run_type, _who);
    }
    void sendSignal_longTaskQuantity(int _quantity)
    {
        emit signal_LongTaskQuantity(_quantity);
    }
    void sendSignal_AverageQuantityOfTasks(int _id, QString _count)
    {
        emit signal_SharedThread_AverageQuantityOfTasks(_id, _count);
    }
    void sendSignal_DeleteLongTask(int _id)
    {
        emit signal_UnsharedThread_DeleteLongTask(_id);
    }
    void sendSignal_DeleteExtraLongTask(int _id)
    {
        emit signal_UnsharedThread_DeleteExtraLongTask(_id);
    }

    void sendSignal_poolNotWork()
    {
        //emit signal_DeleteTopTaskFromThread(id);
    }
    void sendSignal_NoThread()
    {
        //emit signal_DeleteTopTaskFromThread(id);
        addWarningToShell(QString("Thread not find"), eLogWarning::WARNING);
    }

    void sendSignal_AddRepeatTask(int _task_id, int _time, QString _who, int _repeatTaskQuantity)
    {
        emit signal_AddRepeatTask(_task_id, _time, _who, _repeatTaskQuantity);
    }
    void sendSignal_DeleteRepeatTask(int _task_id)
    {
        emit signal_DeleteRepeatTask(_task_id);
    }
    void sendSignal_EditRepeatTaskTime(int _task_id, int _time)
    {
        emit signal_EditRepeatTaskTime(_task_id, _time);
    }

    void sendSignals_AffinityMode(eAffinityMode _affinityMode)
    {
        emit signal_AffinityMode(_affinityMode);
    }

    void sendSignals_Info(int _unshared, int _shared, int _coreQuantity, eAffinityMode _affinityMode)
    {
        emit signal_QuantityUnsharedThreads(_unshared);
        emit signal_QuantitySharedThreads(_shared);
        emit signal_QuantityCores(_coreQuantity);
        emit signal_AffinityMode(_affinityMode);
    }

    void sendSignal_AffinityMask(int _threadNum, int _threadType, int _threadMask)
    {
        emit signal_AffinityMask(_threadNum, _threadType, _threadMask);
    }

    //add new warning in Warning_Table
    void addWarningToShell(QString _message, eLogWarning _value)
    {
        emit signal_AddWarning(_message, _value);
    }

signals:
    void signal_PoolState(int);
    void signal_NewThread(int, int);
    void signal_UnsharedThread_AddLongTask(int, int, QString);
    void signal_AddExtraUnsharedThread(int, int, QString);
    void signal_UnsharedThread_DeleteLongTask(int);
    void signal_UnsharedThread_DeleteExtraLongTask(int);
    void signal_LongTaskQuantity(int);
    void signal_SharedThread_AverageQuantityOfTasks(int, QString);
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
