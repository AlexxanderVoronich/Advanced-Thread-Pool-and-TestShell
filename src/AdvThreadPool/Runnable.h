// FILE Runnable.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef CRUNNABLE_H
#define	CRUNNABLE_H

#include <QString>
#include <functional>
#include <iostream>

enum eRunnableType{EMPTY_TASK=0, LONG_TASK=1, SHORT_TASK, REPEAT_TASK, LONG_TASK_EXTRA};

///meta-programming for creating variadic template-class that described sequence of indexes (0,1,2,3,...)
template<int ...S> struct seq {};
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

/////////ALL TYPES TASKs////////////////////////
///base template class-wrapper (pattern Command)
///
template<typename T,  typename R, typename ...Args>
class CRunnable
{
    friend class CAdvThreadPool;

public://typedefs
    typedef R _RETURN_TYPE;
    typedef R (T::*RunnableFunction)(Args...);
    typedef void (T::*StopRunnableSign)();

private://data
    T* m_pProcessor = nullptr; //pointer to object-container of runnable task
    RunnableFunction m_RunFunction = nullptr;//pointer to runnable task
    StopRunnableSign m_StopFunction = nullptr;//pointer to stop_function of runnable task
    QString m_Description;//description of runnable task
    int m_iRunnableType = EMPTY_TASK;//LONG_TASK,SHORT_TASK or REPEAT_TASK
    std::tuple<Args...> m_Params;//parameters for tranfer to runnable task
    int m_iRepeatTime = 0;//Rerun time for REPEAT_TASK
    bool deleteExtraThreadAfterStopRunnable = true;//only for CLongTask in case EXTRA_THREAD

public://methods
    CRunnable(T* processor,
                RunnableFunction runFunction,
                eRunnableType runType = SHORT_TASK,
                std::tuple<Args...> params  = std::make_tuple(),
                StopRunnableSign stopFunction = nullptr,
                QString description = "",
                int repeatTime = -1)
        :m_pProcessor(processor),
        m_RunFunction(runFunction),
        m_StopFunction(stopFunction),
        m_iRunnableType(runType),
        m_Params(params),
        m_iRepeatTime(repeatTime)
    {
        m_Description = description;
    }

    virtual ~CRunnable(){;}

    void stopRunnable()
    {
        (m_pProcessor->*m_StopFunction)();
    }

    QString getDescription(){return m_Description;}
    int getType(){return m_iRunnableType;}
    bool updateLongTypeToExtra()
    {
        if(m_iRunnableType == eRunnableType::LONG_TASK)
        {
            m_iRunnableType = eRunnableType::LONG_TASK_EXTRA;
            return true;
        }
        else
            return false;
    }
    int getRepeatTime(){return m_iRepeatTime;}
    T* getProcessor(){return m_pProcessor;}
    void setDeleteExtraThreadSign(bool sign){deleteExtraThreadAfterStopRunnable = sign;}
    bool getDeleteExtraThreadSign(){return deleteExtraThreadAfterStopRunnable;}

private://methods
    int run()
    {
        return callThread(typename gens<sizeof...(Args)>::type());
    }

    template<int ...S>
    int callThread(seq<S...>)
    {
        return (m_pProcessor->*m_RunFunction)(std::get<S>(m_Params) ...);
    }

};

//////SHORT_TASK///////////////////////////////////
///derived template class-wrapper (pattern Command)
///
template<typename T, typename R, typename ...Args>
class CShortTask: public CRunnable<T, R, Args...>
{
public://methods
    CShortTask(T* processor,
                typename CRunnable<T,R,Args...>::RunnableFunction runFunction,
                QString description = "Short task",
                std::tuple<Args...> params = std::make_tuple())
        : CRunnable<T,R,Args...>(processor, runFunction, SHORT_TASK, params, nullptr, description, -1)
    {}
};

//////REPEAT_TASK///////////////////////////////////
///derived template class-wrapper (pattern Command)
///
template<typename T, typename R, typename ...Args>
class CRepeatTask: public CRunnable<T, R, Args...>
{
public://methods
    CRepeatTask(T* processor,
                typename CRunnable<T,R,Args...>::RunnableFunction runFunction,
                int repeatTime,
                QString description = "Repeat task",
                std::tuple<Args...> params = std::make_tuple())
        : CRunnable<T,R,Args...>(processor, runFunction, REPEAT_TASK, params, nullptr, description, repeatTime)
    {}
};

//////LONG_TASK///////////////////////////////////
///derived template class-wrapper (pattern Command)
///see macros_CreateLongTask in AdvMacros.h
template<typename T, typename R, typename ...Args>
class CLongTask: public CRunnable<T, R, Args...>
{
public://methods
    CLongTask(T* processor,
                typename CRunnable<T, R, Args...>::RunnableFunction runFunction,
                typename CRunnable<T, R, Args...>::StopRunnableSign stopFunction,
                QString description = "Long task",
                std::tuple<Args...> params = std::make_tuple())
        : CRunnable<T,R, Args...>(processor, runFunction, LONG_TASK, params, stopFunction, description, -1)
    {}
};

#endif	/* CRUNNABLE_H */

