// FILE AdvMacros.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef ADVMACROS_H
#define ADVMACROS_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////Macroses for creation of each type tasks///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// task - task that will be create
/// taskClassPtr - pointer on object that contain RunFunction
/// taskClassName - name of class that contain RunFunction
/// taskRunFunction - name of RunFunction in specified class
/// taskReturnType - return type for task
/// taskStopFunction - name of StopFunctiom in specified class (only for long tasks)
/// taskDescription - string task desccription
/// taskPeriod - time for task restarting (only for repeat tasks)

#define macros_CreateLongTask(taskClassPtr, taskClassName, taskRunFunction, taskReturnType, taskStopFunction, taskDescription) \
            new cLongTask<taskClassName, taskReturnType>(taskClassPtr, \
                &taskClassName::taskRunFunction, \
                &taskClassName::taskStopFunction, \
                taskDescription);

#define macros_CreateRepeatTask(taskClassPtr, taskClassName, taskRunFunction, taskReturnType, taskPeriod, taskDescription) \
            new cRepeatTask<taskClassName, taskReturnType>(taskClassPtr, \
                &taskClassName::taskRunFunction, \
                taskPeriod, \
                taskDescription);

#define macros_CreateShortTask(taskClassPtr, taskClassName, taskRunFunction, taskReturnType) \
            new cShortTask<taskClassName, taskReturnType>(taskClassPtr, &taskClassName::taskRunFunction);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////Macroses for creation and launch of each type tasks///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// task - task that will be create
/// taskClassPtr - pointer on object that contain RunFunction
/// taskClassName - name of class that contain RunFunction
/// taskRunFunction - name of RunFunction in specified class
/// taskReturnType - return type for task
/// taskStopFunction - name of StopFunctiom in specified class (only for long tasks)
/// taskDescription - string task desccription
/// taskPeriod - time for task restarting (only for repeat tasks)


#define macros_CreateAndLaunchLongTask(task, taskClassPtr, taskClassName, taskRunFunction, taskReturnType, taskStopFunction, taskDescription) \
            macros_CreateLongTask(taskClassPtr, taskClassName, taskRunFunction, taskReturnType, taskStopFunction, taskDescription); \
            auto future_of_##task = cAdvThreadPool::launchRunnableObject<taskReturnType, cLongTask<taskClassName, taskReturnType> >(task);

#define macros_CreateAndLaunchRepeatTask(task, taskClassPtr, taskClassName, taskRunFunction, taskReturnType, taskPeriod, taskDescription) \
            macros_CreateRepeatTask(taskClassPtr, taskClassName, taskRunFunction, taskReturnType, taskPeriod, taskDescription); \
            auto future_of_##task = cAdvThreadPool::launchRunnableObject<taskReturnType, cRepeatTask<taskClassName, taskReturnType>>(task);

#define macros_CreateAndLaunchShortTask(task, taskClassPtr, taskClassName, taskRunFunction, taskReturnType) \
            macros_CreateShortTask(taskClassPtr, taskClassName, taskRunFunction, taskReturnType); \
            auto future_of_##task = cAdvThreadPool::launchRunnableObject<taskReturnType, cShortTask<taskClassName, taskReturnType>>(task);

#endif // ADVMACROS_H
