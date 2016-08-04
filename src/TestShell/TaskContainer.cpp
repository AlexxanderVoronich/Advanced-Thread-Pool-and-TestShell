#include "TaskContainer.h"
#include <thread>
#include <chrono>

Tasks::Tasks()
{

}

int Tasks::repeatTaskFunction()
{
    return 1000;
}

LongTaskContainer::LongTaskContainer(QString text)
{
    description = text;
    runnableSign = true;
    std::cout << "LongTaskContainer constructor for "<<description.toStdString()<< std::endl;
}

LongTaskContainer::~LongTaskContainer()
{
    runnableSign = false;
    std::cout << "LongTaskContainer destructor for "<<description.toStdString()<< std::endl;
}

qint32 LongTaskContainer::longTaskFunction()
{
    while(runnableSign)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

void LongTaskContainer::stopLongTaskFunction()
{
    runnableSign = false;
}

ShortTaskGenerator::ShortTaskGenerator()
{
    std::cout << "ShorTaskGenerator constructor" << std::endl;
}

ShortTaskGenerator::~ShortTaskGenerator()
{
    runnableSign = false;
    std::cout << "ShorTaskGenerator destructor" << std::endl;
}

qint32 ShortTaskGenerator::generatorMainFunction(qint32 shortTaskQuantity)
{
    while(runnableSign)
    {
        for(int i=0; i<shortTaskQuantity; ++i)
        {
            //1)create new short task (usual variant)
            QString nameShortTask = QString("Short Task %1").arg(i);
            auto tupleWithArguments = std::make_tuple<>(nameShortTask);
            auto pShortTask = new CShortTask<ShortTaskGenerator, qint32, QString>(this,
                                                    &ShortTaskGenerator::shortTaskFunction,
                                                    nameShortTask,
                                                    tupleWithArguments);
            auto future_of_repeatTask = CAdvThreadPool::launchRunnableObject<qint32, CShortTask<ShortTaskGenerator, qint32, QString>>(pShortTask);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

qint32 ShortTaskGenerator::shortTaskFunction(QString taskName)
{

}

void ShortTaskGenerator::stopGenerator()
{
    runnableSign = false;
}

bool ShortTaskGenerator::start(qint32 shortTaskQuantity)
{
    if(runnableSign)
        return false;

    runnableSign = true;
    auto tupleWithArguments = std::make_tuple<>(shortTaskQuantity);
    auto pGeneratorMainTask = new CLongTask<ShortTaskGenerator, qint32, qint32>(this,
                                            &ShortTaskGenerator::generatorMainFunction,
                                            &ShortTaskGenerator::stopGenerator,
                                            QString("Generator Main Task"),
                                            tupleWithArguments);

    auto pFuture = CAdvThreadPool::launchRunnableObject<qint32, CLongTask<ShortTaskGenerator, qint32, qint32>>(pGeneratorMainTask);
    if(pFuture == nullptr)
        return false;
    else
        return true;
}
