#ifndef _DLLHEADER_H_
#define _DLLHEADER_H_ 

#include <stdio.h>
#include <windows.h> 
#include <string>
#include <memory>

#if defined( BUILD_DLL )
#define IMPORT_EXPORT __declspec(dllexport)
#else
#define IMPORT_EXPORT __declspec(dllimport)
#endif

extern "C" IMPORT_EXPORT void openPool();

typedef void (CALLBACK *FPTR)(void);
class cAdvPoolGUI;
class cDllQtApplicationTaskContainer;

// This is an exported class.
class IMPORT_EXPORT cDllThreadPool
{
public:
    static void startThreadPool(int _unsharedThreadsQuantity,
        int _sharedThreadsQuantity,
        int _task_mode,
        int _stretch_mode,
        std::string _file);

    static void stopThreadPool();

    static int createAndLaunchLongTask(std::string _longTaskName, FPTR _run_delegate, FPTR _stop_delegate);
    static int createAndLaunchRepeatTask(FPTR _run_delegate, FPTR _stop_delegate);

private:
    static std::shared_ptr<cDllQtApplicationTaskContainer> m_qt_app;
};

class cDllLongTaskContainer
{
public:
    cDllLongTaskContainer(FPTR _run_delegate, FPTR _stop_delegate)
    {
        m_callback_run = _run_delegate;
        m_callback_stop = _stop_delegate;
    }

    int longTaskFunction()
    {
        if(m_callback_run != nullptr)
        {
            m_callback_run();
        }
        return 0;
    }
    void stopLongTaskFunction()
    {
        if (m_callback_stop != nullptr)
        {
            m_callback_stop();
        }
    }

    FPTR m_callback_run = nullptr;
    FPTR m_callback_stop = nullptr;

};


class cDllQtApplicationTaskContainer
{
public:
    cDllQtApplicationTaskContainer();
    int longTaskFunction();
    void stopLongTaskFunction();
};

#endif