// FILE ThreadPoolSettings.cpp
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "ThreadPoolSettings.h"
#include "qobjectserializer.h"

/*
template<class Archive>
void CThreadPoolSettings::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(quantitySharedThreads);
    ar & BOOST_SERIALIZATION_NVP(quantityUnsharedThreads);
    ar & BOOST_SERIALIZATION_NVP(affinityMode);

    if(Archive::is_loading::value)//if(m_bSignRead)
    {
        delete [] maskArray;
        maskArray = nullptr;
        maskArray = new int[quantitySharedThreads+quantityUnsharedThreads];
    }
    try
    {
        ar & boost::serialization::make_array<int>(maskArray, quantitySharedThreads+quantityUnsharedThreads);
        //boost::serialization::make_nvp("Vector", m_MaskArray);
    }
    catch(std::exception& e)
    {
        throw maskArray_Exception("ExceptMaskArray");
    }

}
*/

bool cThreadPoolSettings::isValid()
{
    return !(m_quantitySharedThreads == 0 && m_quantityUnsharedThreads == 0);
}

void cThreadPoolSettings::saveArchive()
{
    /*try //serialization based on libBoostSerialization
    { // serialize
        // std::ofstream ofs(file.c_str(), std::ios::out);
        isRead = false;
        std::ofstream ofs(qPrintable(filePath), std::ios::out);
        boost::archive::xml_oarchive oa(ofs);
        oa << boost::serialization::make_nvp("Thread_Pool_Settings", *this);
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<" - CThreadPoolSettings::SaveArch() "<<std::endl;
    }*/

    //serialization based on qt meta-object information and QDom
    QObjectSerializer::serialize(m_filePath, this);
}

bool cThreadPoolSettings::readArchive()
{
    /*try //deserialization based on libBoostSerialization
    { // deserialize
        std::ifstream ifs(qPrintable(filePath), std::ios::in);
        if ( ifs.is_open())
        {
            isRead = true;
            boost::archive::xml_iarchive ia(ifs);
            ia >> boost::serialization::make_nvp("Thread_Pool_Settings", *this);
            return true;
        }
        else
            return false;

    }
    catch(const maskArray_Exception& e)
    {
        std::cout<<e.what()<<" - CThreadPoolSettings::ReadArch() "<<std::endl;
        return true;
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<" - CThreadPoolSettings::ReadArch() "<<std::endl;
        return false;
    }*/

    //deserialization based on qt meta-object information and QDom
    return QObjectSerializer::deserialize(m_filePath, this);
}
