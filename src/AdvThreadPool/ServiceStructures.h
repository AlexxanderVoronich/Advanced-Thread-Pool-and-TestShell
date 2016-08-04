// FILE ServiceStructures.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef SERVICESTRUCTURES_H
#define SERVICESTRUCTURES_H

#include <QDockWidget>
#include <QElapsedTimer>
#include <functional>

enum class eLogWarning{warning = 0, message};
enum class eAffinityMode{No_Affinity = 1, Yes_Affinity, No_Affinity_Without_GUI_Edition, Yes_Affinity_Without_GUI_Edition};
enum class eThreadPoolMode{ORDINARY_MODE, REPEATED_TASK_MODE};
enum class eStretchMode{NO_STRETCH, YES_STRETCH};

Q_DECLARE_METATYPE(eLogWarning)
Q_DECLARE_METATYPE(eAffinityMode)

class CPoolModes
{
public://data
    //ORDINARY_MODE or USE_REPEATED_TASK_MODE(create of system runnable task)
    eThreadPoolMode threadPoolMode = eThreadPoolMode::ORDINARY_MODE;//field is not serialized
    eAffinityMode affinityMode = eAffinityMode::No_Affinity;//field is serialized
    eStretchMode stretchMode = eStretchMode::NO_STRETCH;//field is not serialized

public://methods
    CPoolModes& operator = (const CPoolModes& toCopy)
    {
        threadPoolMode = toCopy.threadPoolMode;
        affinityMode = toCopy.affinityMode;
        stretchMode = toCopy.stretchMode;
        return *this;
    }
};



class holdOverTask_Exception
{
private:
    std::string msg;
    int holdOverInterval = 0;
public:
    holdOverTask_Exception(const std::string& msg):msg(msg), holdOverInterval(0){}
    holdOverTask_Exception(const std::string& msg, int interval):msg(msg), holdOverInterval(interval){}
    std::string what()const{return msg;}
    int getInterval()const{return holdOverInterval;}
};

template<class _T, class _RUNNABLE>
struct CAdvFuture
{
    CAdvFuture():ready(false), taskDescription(""){}
    ~CAdvFuture(){}

public://data
    //ptr to runnable_task (will be deleted in the pool after execution's end)
    _RUNNABLE* pTask = nullptr;
    void* pHostObject = nullptr;//ptr to HostObject
    bool ready = false;//sign of task execution
    _T data;//some data
    QString taskDescription;
    int executionCounter = 3;
    int taskType = 0;
    int repeatTaskID = 0;
    int timeIntervalForHoldOverShortTask = 0;
    QElapsedTimer qt_timer;
    bool resultIsTimerOver = false;
};

typedef std::function<QString(int, int)> runnable_closure;

class CRepeatedTaskAdapter
{
public:
    CRepeatedTaskAdapter(runnable_closure task):run(task){}
    runnable_closure run;
    int ID = 0;
    int timePeriod = 0;
    QElapsedTimer qt_timer;
};

class CPoolDockWidget: public QDockWidget
{
    Q_OBJECT

public:
    CPoolDockWidget(QWidget *widget):QDockWidget(widget){}
    virtual ~CPoolDockWidget(){}
};

struct SAffinityData
{
public:
    int coreMask = 0;
    int coreChanged = 0;
};


#endif // SERVICESTRUCTURES_H
