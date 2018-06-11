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

class cThreadPoolSettings: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int quantitySharedThreads_property READ getQuantitySharedThreads WRITE setQuantitySharedThreads)
    Q_PROPERTY(int quantityUnsharedThreads_property READ getQuantityUnsharedThreads WRITE setQuantityUnsharedThreads)
    Q_PROPERTY(eAffinityMode affinityMode_property READ getAffinityMode WRITE setAffinityMode)
    Q_PROPERTY(std::vector<int> masks_property READ getMasks WRITE setMasks)

public://fields
    QString m_filePath;
    bool m_isPoolStarted = false;//Sign of pool's starting
    int ID_TASK_COUNTER = 0;//ID of repeated short task
    int m_coreQuantity = 0;
    std::vector<int> m_masks;
    bool m_isRead = false;
    cPoolModes m_poolModes;

private://fields
    int m_quantitySharedThreads = 0;
    int m_quantityUnsharedThreads = 0;

public://methods
    bool isValid();
    //template<class Archive> void serialize(Archive & ar, const unsigned int version);
    void saveArchive();
    bool readArchive();

    int getQuantitySharedThreads(){return m_quantitySharedThreads;}
    void setQuantitySharedThreads(int value){m_quantitySharedThreads = value;}

    int getQuantityUnsharedThreads(){return m_quantityUnsharedThreads;}
    void setQuantityUnsharedThreads(int value){m_quantityUnsharedThreads = value;}

    eAffinityMode getAffinityMode(){return m_poolModes.m_affinityMode;}
    void setAffinityMode(eAffinityMode value)
    {
        m_poolModes.m_affinityMode = value;
    }

    std::vector<int>& getMasks(){return m_masks;}
    void setMasks(std::vector<int> _value){m_masks = _value;}
};

#endif // THREADPOOLSETTINGS

