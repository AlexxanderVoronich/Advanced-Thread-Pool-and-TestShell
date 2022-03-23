#ifndef TASKCONTAINER_H
#define TASKCONTAINER_H
#include <qglobal.h>
#include <list>
#include <QString>
#include <memory>
#include <AdvThreadPool/ServiceStructures.h>
#include <AdvThreadPool/Runnable.h>

class LongTaskContainer
{
    bool m_runnableSign = false;
    QString m_description;

public:
    LongTaskContainer(QString _text);
    ~LongTaskContainer();

    qint32 longTaskFunction();
    void stopLongTaskFunction();
};

class ShortTaskGenerator
{
    bool m_runnableSign = false;
public:
    ShortTaskGenerator();
    ~ShortTaskGenerator();

    qint32 generate(qint32 shortTaskQuantity);
    qint32 shortTaskFunction(QString taskName);

    void stop();
    bool start(qint32 shortTaskQuantity);
};

class Tasks
{
public://data
    //list with futures of long tasks
    std::list<std::shared_ptr<cAdvFuture<qint32, cLongTask<LongTaskContainer, qint32>>>> m_longTaskFutures;
    //list with futures of repeat tasks
    std::list<std::shared_ptr<cAdvFuture<qint32, cRepeatTask<Tasks, qint32>>>> m_repeatTaskFutures;

public://methods
    Tasks();
    int repeatTaskFunction();
};

#endif // TASKCONTAINER_H
