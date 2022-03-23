#include "TaskContainer.h"
#include <thread>
#include <chrono>
#include <iostream>
#include "AdvThreadPool/AdvThreadPool.h"

Tasks::Tasks()
{

}

int Tasks::repeatTaskFunction()
{
    return 1000;
}

LongTaskContainer::LongTaskContainer(QString text)
{
    m_description = text;
    m_runnableSign = true;
    std::cout << "LongTaskContainer constructor for "<<m_description.toStdString()<< std::endl;
}

LongTaskContainer::~LongTaskContainer()
{
    m_runnableSign = false;
    std::cout << "LongTaskContainer destructor for "<<m_description.toStdString()<< std::endl;
}

qint32 LongTaskContainer::longTaskFunction()
{
    while(m_runnableSign)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

void LongTaskContainer::stopLongTaskFunction()
{
    m_runnableSign = false;
}

ShortTaskGenerator::ShortTaskGenerator()
{
    std::cout << "ShortTaskGenerator constructor" << std::endl;
}

ShortTaskGenerator::~ShortTaskGenerator()
{
    m_runnableSign = false;
    std::cout << "ShortTaskGenerator destructor" << std::endl;
}

qint32 ShortTaskGenerator::generate([[maybe_unused]] qint32 _shortTaskQuantity)
{
    while(m_runnableSign)
    {
        for(int i=0; i<_shortTaskQuantity; ++i)
        {
            //1)create new short task (usual variant)
            QString nameShortTask = QString("Short Task %1").arg(i);
            auto tupleWithArguments = std::make_tuple<>(nameShortTask);
            auto pShortTask = new cShortTask<ShortTaskGenerator, qint32, QString>(this,
                                                    &ShortTaskGenerator::shortTaskFunction,
                                                    nameShortTask,
                                                    tupleWithArguments);
            auto future_of_repeatTask = cAdvThreadPool::launchRunnableObject<qint32, cShortTask<ShortTaskGenerator, qint32, QString>>(pShortTask);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

qint32 ShortTaskGenerator::shortTaskFunction([[maybe_unused]] QString _taskName)
{
    std::cout << "ShortTaskFunction:" << _taskName.toStdString() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return 0;
}

void ShortTaskGenerator::stop()
{
    m_runnableSign = false;
}

bool ShortTaskGenerator::start([[maybe_unused]] qint32 _shortTaskQuantity)
{
    if(m_runnableSign)
        return false;

    m_runnableSign = true;
    auto tupleWithArguments = std::make_tuple<>(_shortTaskQuantity);
    auto pGeneratorMainTask = new cLongTask<ShortTaskGenerator, qint32, qint32>(this,
                                            &ShortTaskGenerator::generate,
                                            &ShortTaskGenerator::stop,
                                            QString("Generator Main Task"),
                                            tupleWithArguments);

    auto pFuture = cAdvThreadPool::launchRunnableObject<qint32, cLongTask<ShortTaskGenerator, qint32, qint32>>(pGeneratorMainTask);
    if(pFuture == nullptr)
        return false;
    else
        return true;

    return false;
}
