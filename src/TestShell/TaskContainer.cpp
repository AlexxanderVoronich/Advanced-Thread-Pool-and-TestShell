#include "TaskContainer.h"
#include <thread>
#include <chrono>
#include <iostream>

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
    std::cout << "ShorTaskGenerator constructor" << std::endl;
}

ShortTaskGenerator::~ShortTaskGenerator()
{
    m_runnableSign = false;
    std::cout << "ShorTaskGenerator destructor" << std::endl;
}

qint32 ShortTaskGenerator::generatorMainFunction(qint32 shortTaskQuantity)
{
    /*while(m_runnableSign)
    {
        for(int i=0; i<shortTaskQuantity; ++i)
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
    }*/
    return 0;
}

qint32 ShortTaskGenerator::shortTaskFunction(QString taskName)
{
    return 0;
}

void ShortTaskGenerator::stopGenerator()
{
    m_runnableSign = false;
}

bool ShortTaskGenerator::start(qint32 shortTaskQuantity)
{
    if(m_runnableSign)
        return false;

    /*m_runnableSign = true;
    auto tupleWithArguments = std::make_tuple<>(shortTaskQuantity);
    auto pGeneratorMainTask = new cLongTask<ShortTaskGenerator, qint32, qint32>(this,
                                            &ShortTaskGenerator::generatorMainFunction,
                                            &ShortTaskGenerator::stopGenerator,
                                            QString("Generator Main Task"),
                                            tupleWithArguments);

    auto pFuture = cAdvThreadPool::launchRunnableObject<qint32, cLongTask<ShortTaskGenerator, qint32, qint32>>(pGeneratorMainTask);
    if(pFuture == nullptr)
        return false;
    else
        return true;*/
}
