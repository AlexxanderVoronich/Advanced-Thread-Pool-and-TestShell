// FILE ThreadPoolSettings.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef THREADPOOLSETTINGS
#define THREADPOOLSETTINGS

#include <QObject>
#include <QString>
#include <vector>
#include "ServiceStructures.h"

class CThreadPoolSettings: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int quantitySharedThreads_property READ getQuantitySharedThreads WRITE setQuantitySharedThreads)
    Q_PROPERTY(int quantityUnsharedThreads_property READ getQuantityUnsharedThreads WRITE setQuantityUnsharedThreads)
    Q_PROPERTY(eAffinityMode affinityMode_property READ getAffinityMode WRITE setAffinityMode)
    Q_PROPERTY(std::vector<int> masks_property READ getMasks WRITE setMasks)

public://fields
    QString filePath;
    bool isPoolStarted = false;//Sign of pool's starting
    int ID_TASK_COUNTER = 0;//ID of repeated short task
    int coreQuantity = 0;
    std::vector<int> masks;
    bool isRead = false;
    CPoolModes poolModes;

private://fields
    int quantitySharedThreads = 0;
    int quantityUnsharedThreads = 0;    

public://methods
    bool isValid();
    //template<class Archive> void serialize(Archive & ar, const unsigned int version);
    void saveArchive();
    bool readArchive();

    int getQuantitySharedThreads(){return quantitySharedThreads;}
    void setQuantitySharedThreads(int value){quantitySharedThreads = value;}

    int getQuantityUnsharedThreads(){return quantityUnsharedThreads;}
    void setQuantityUnsharedThreads(int value){quantityUnsharedThreads = value;}

    eAffinityMode getAffinityMode(){return poolModes.affinityMode;}
    void setAffinityMode(eAffinityMode value)
    {
        poolModes.affinityMode = value;
    }

    std::vector<int>& getMasks(){return masks;}
    void setMasks(std::vector<int> value){masks = value;}
};

#endif // THREADPOOLSETTINGS

