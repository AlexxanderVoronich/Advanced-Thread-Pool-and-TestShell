// FILE ServiceStructures.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef SERVICESTRUCTURES_H
#define SERVICESTRUCTURES_H

#include <QDockWidget>
#include <QElapsedTimer>
#include <functional>
#include "DllHeader.h"

enum class eLogWarning{WARNING = 0, MESSAGE};
enum class eAffinityMode{NO_AFFINITY = 1, YES_AFFINITY, No_Affinity_Without_GUI_Edition, Yes_Affinity_Without_GUI_Edition};
enum class eThreadPoolMode{ORDINARY_MODE, REPEATED_TASK_MODE};
enum class eStretchMode{NO_STRETCH, YES_STRETCH};

Q_DECLARE_METATYPE(eLogWarning)
Q_DECLARE_METATYPE(eAffinityMode)

class IMPORT_EXPORT cPoolModes
{
public://data
    //ORDINARY_MODE or USE_REPEATED_TASK_MODE(create of system runnable task)
    eThreadPoolMode m_threadPoolMode = eThreadPoolMode::ORDINARY_MODE;//field is not serialized
    eAffinityMode m_affinityMode = eAffinityMode::NO_AFFINITY;//field is serialized
    eStretchMode m_stretchMode = eStretchMode::NO_STRETCH;//field is not serialized

public://methods
    cPoolModes& operator = (const cPoolModes& toCopy)
    {
        m_threadPoolMode = toCopy.m_threadPoolMode;
        m_affinityMode = toCopy.m_affinityMode;
        m_stretchMode = toCopy.m_stretchMode;
        return *this;
    }
};

class ExceptionHoldOverTask
{
private:
    std::string m_message;
    int m_holdOverInterval = 0;
public:
    ExceptionHoldOverTask(const std::string& _msg):m_message(_msg), m_holdOverInterval(0){}
    ExceptionHoldOverTask(const std::string& _msg, int _interval):m_message(_msg), m_holdOverInterval(_interval){}
    std::string what()const{return m_message;}
    int getInterval()const{return m_holdOverInterval;}
};

template<class _T, class _RUNNABLE>
struct cAdvFuture
{
    cAdvFuture():m_ready(false), m_taskDescription(""){}
    ~cAdvFuture(){}

public://data
    //ptr to runnable_task (will be deleted in the pool after execution's end)
    _RUNNABLE* m_task = nullptr;
    void* m_hostObject = nullptr;//ptr to HostObject
    bool m_ready = false;//sign of task execution
    _T m_data;//some data
    QString m_taskDescription;
    int m_executionCounter = 3;
    int m_taskType = 0;
    int m_repeatTaskId = 0;
    int m_timeIntervalForHoldOverShortTask = 0;
    QElapsedTimer m_qt_timer;
    bool m_resultIsTimerOver = false;
};

typedef std::function<QString(int, int)> runnable_closure;

class cRepeatedTaskAdapter
{
public:
    cRepeatedTaskAdapter(runnable_closure _task):m_run(_task){}
    runnable_closure m_run;
    int m_id = 0;
    int m_timePeriod = 0;
    QElapsedTimer m_qt_timer;
};

class cPoolDockWidget: public QDockWidget
{
    Q_OBJECT

public:
    cPoolDockWidget(QWidget *_widget):QDockWidget(_widget){}
    virtual ~cPoolDockWidget(){}
};

struct SAffinityData
{
public:
    int m_coreMask = 0;
    int m_coreChanged = 0;
};


#endif // SERVICESTRUCTURES_H
