#ifndef TASKCONTAINER_H
#define TASKCONTAINER_H
#include <qglobal.h>
#include <list>
#include "AdvThreadPool/AdvThreadPool.h"

class LongTaskContainer
{
    bool runnableSign = false;
    QString description;
public:
    LongTaskContainer(QString text);
    ~LongTaskContainer();

    qint32 longTaskFunction();
    void stopLongTaskFunction();
};

class ShortTaskGenerator
{
    bool runnableSign = false;
public:
    ShortTaskGenerator();
    ~ShortTaskGenerator();

    qint32 generatorMainFunction(qint32 shortTaskQuantity);
    qint32 shortTaskFunction(QString taskName);

    void stopGenerator();
    bool start(qint32 shortTaskQuantity);
};

class Tasks
{
public://data
    //list with futures of long tasks
    std::list<std::shared_ptr<CAdvFuture<qint32, CLongTask<LongTaskContainer, qint32>>>> longTaskFutures;
    //list with futures of repeat tasks
    std::list<std::shared_ptr<CAdvFuture<qint32, CRepeatTask<Tasks, qint32>>>> repeatTaskFutures;

public://methods
    Tasks();
    int repeatTaskFunction();
};

#endif // TASKCONTAINER_H
