// FILE Runnable.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef CRUNNABLE_H
#define	CRUNNABLE_H

#include <QString>
#include <functional>
#include <iostream>
#include "DllHeader.h"

enum eRunnableType{EMPTY_TASK=0, LONG_TASK=1, SHORT_TASK, REPEAT_TASK, LONG_TASK_EXTRA};

///meta-programming for creating variadic template-class that described sequence of indexes (0,1,2,3,...)
template<int ...S> struct seq {};
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

/////////ALL TYPES TASKs////////////////////////
///base template class-wrapper (pattern Command)
///
template<typename T,  typename R, typename ...Args>
class cRunnable
{
    friend class cAdvThreadPool;

public://typedefs
    typedef R _RETURN_TYPE;
    typedef R (T::*RunnableFunction)(Args...);
    typedef void (T::*StopRunnableSign)();

private://data
    T* m_processor = nullptr; //pointer to object-container of runnable task
    RunnableFunction m_runFunction = nullptr;//pointer to runnable task
    StopRunnableSign m_stopFunction = nullptr;//pointer to stop_function of runnable task
    QString m_description;//description of runnable task
    int m_runnableType = EMPTY_TASK;//LONG_TASK,SHORT_TASK or REPEAT_TASK
    std::tuple<Args...> m_params;//parameters for tranfer to runnable task
    int m_repeatTime = 0;//Rerun time for REPEAT_TASK
    bool m_deleteExtraThreadAfterStopRunnable = true;//only for CLongTask in case EXTRA_THREAD

public://methods
    cRunnable(T* _processor,
                RunnableFunction _runFunction,
                eRunnableType _runType = SHORT_TASK,
                std::tuple<Args...> _params  = std::make_tuple(),
                StopRunnableSign _stopFunction = nullptr,
                QString _description = "",
                int _repeatTime = -1)
        :m_processor(_processor),
        m_runFunction(_runFunction),
        m_stopFunction(_stopFunction),
        m_runnableType(_runType),
        m_params(_params),
        m_repeatTime(_repeatTime)
    {
        m_description = _description;
    }

    virtual ~cRunnable(){;}

    void stopRunnable()
    {
        (m_processor->*m_stopFunction)();
    }

    QString getDescription(){return m_description;}
    int getType(){return m_runnableType;}
    bool updateLongTypeToExtra()
    {
        if(m_runnableType == eRunnableType::LONG_TASK)
        {
            m_runnableType = eRunnableType::LONG_TASK_EXTRA;
            return true;
        }
        else
            return false;
    }
    int getRepeatTime(){return m_repeatTime;}
    T* getProcessor(){return m_processor;}
    void setDeleteExtraThreadSign(bool _sign){m_deleteExtraThreadAfterStopRunnable = _sign;}
    bool getDeleteExtraThreadSign(){return m_deleteExtraThreadAfterStopRunnable;}

private://methods
    int run()
    {
        return callThread(typename gens<sizeof...(Args)>::type());
    }

    template<int ...S>
    int callThread(seq<S...>)
    {
        return (m_processor->*m_runFunction)(std::get<S>(m_params) ...);
    }

};

//////SHORT_TASK///////////////////////////////////
///derived template class-wrapper (pattern Command)
///
template<typename T, typename R, typename ...Args>
class cShortTask: public cRunnable<T, R, Args...>
{
public://methods
    cShortTask(T* _processor,
                typename cRunnable<T,R,Args...>::RunnableFunction _runFunction,
                QString _description = "Short task",
                std::tuple<Args...> _params = std::make_tuple())
        : cRunnable<T,R,Args...>(_processor, _runFunction, SHORT_TASK, _params, nullptr, _description, -1)
    {}
};

//////REPEAT_TASK///////////////////////////////////
///derived template class-wrapper (pattern Command)
///
template<typename T, typename R, typename ...Args>
class cRepeatTask: public cRunnable<T, R, Args...>
{
public://methods
    cRepeatTask(T* _processor,
                typename cRunnable<T,R,Args...>::RunnableFunction _runFunction,
                int _repeatTime,
                QString _description = "Repeat task",
                std::tuple<Args...> _params = std::make_tuple())
        : cRunnable<T,R,Args...>(_processor, _runFunction, REPEAT_TASK, _params, nullptr, _description, _repeatTime)
    {}
};

//////LONG_TASK///////////////////////////////////
///derived template class-wrapper (pattern Command)
///see macros_CreateLongTask in AdvMacros.h
template<typename T, typename R, typename ...Args>
class cLongTask: public cRunnable<T, R, Args...>
{
public://methods
    cLongTask(T* processor,
                typename cRunnable<T, R, Args...>::RunnableFunction _runFunction,
                typename cRunnable<T, R, Args...>::StopRunnableSign _stopFunction,
                QString _description = "Long task",
                std::tuple<Args...> params = std::make_tuple())
        : cRunnable<T,R, Args...>(processor, _runFunction, LONG_TASK, params, _stopFunction, _description, -1)
    {}
};

#endif	/* CRUNNABLE_H */

