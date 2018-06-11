#include "QObjectSerializer.h"
#include "ServiceStructures.h"

QObjectSerializer::QObjectSerializer()
{

}

bool QObjectSerializer::serialize(QString _filePath, QObject *_object)
{
    try
    {
        int VECTOR_INT_ID = qRegisterMetaType<std::vector<int>>("std::vector<int>");
        int eAffinityMode_ID = qRegisterMetaType<eAffinityMode>();

        QDomDocument doc;
        QDomElement root = doc.createElement(_object->metaObject()->className());
        doc.appendChild(root);

        for(int i = 0; i < _object->metaObject()->propertyCount(); i++)
        {
            QMetaProperty prop = _object->metaObject()->property(i);
            QString propertyName = prop.name();
            if(propertyName == "objectName")
                continue;
            QDomElement el = doc.createElement(propertyName);
            QVariant value = _object->property(qPrintable(propertyName));

            if(value.userType() == VECTOR_INT_ID)
            {
               std::vector<int> array = value.value<std::vector<int>>();
               int i=0;
               for(int mask: array)
               {
                   QDomElement item = doc.createElement(QString("Item_%1").arg(i));
                   QDomText item_txt = doc.createTextNode(QString::number(mask));
                   item.appendChild(item_txt);
                   el.appendChild(item);
                   ++i;
               }
               root.appendChild(el);
            }
            else if(value.userType() == eAffinityMode_ID)
            {
                eAffinityMode mode = value.value<eAffinityMode>();
                QDomText txt = doc.createTextNode( QString("%1").arg(int(mode)) );
                el.appendChild(txt);
                root.appendChild(el);
            }
            else
            {
                QDomText txt = doc.createTextNode( value.toString() );
                el.appendChild(txt);
                root.appendChild(el);
            }
        }

        QFile outputFile(_filePath);
        if(outputFile.open(QFile::WriteOnly | QFile::Truncate))
        {
            QTextStream outputStream(&outputFile);
            doc.save(outputStream, QDomNode::EncodingFromTextStream);
            outputFile.close();
        }
        else
            return false;
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<" - QObjectSerializer::serialize() "<<std::endl;
        return false;
    }
    return true;
}

bool QObjectSerializer::deserialize(QString _filePath, QObject *_object)
{
    try
    {
        int VECTOR_INT_ID = qRegisterMetaType<std::vector<int>>("std::vector<int>");
        int eAffinityMode_ID = qRegisterMetaType<eAffinityMode>();

        QFile inputFile(_filePath);
        QDomDocument doc;
        if (!doc.setContent(&inputFile))
            return false;
        QDomElement root = doc.documentElement();
        for(int i = 0; i < _object->metaObject()->propertyCount(); i++)
        {
            QMetaProperty prop = _object->metaObject()->property(i);
            QString propName = prop.name();
            if(propName == "objectName")
                continue;

            QVariant value = _object->property(qPrintable(propName));

            QDomNodeList nodeList = root.elementsByTagName(propName);
            if(nodeList.length() < 1)
                continue;

            QDomNode node = nodeList.at(0);

            if(value.userType() == VECTOR_INT_ID)
            {
                std::vector<int> array;
                QDomNode n = node.firstChild();
                while(!n.isNull())
                {
                    if(n.isElement())
                    {
                        QDomElement e = n.toElement();
                        int num = e.text().toInt();
                        std::cout << QString("Element name: %1 - element value: %2\n").arg(e.tagName()).arg(num).toStdString();
                        array.push_back(num);
                    }
                    n = n.nextSibling();
                }

                QVariant variant;
                variant.setValue(array);
                _object->setProperty(qPrintable(propName), variant);
            }
            else if(value.userType() == eAffinityMode_ID)
            {
                int v = node.toElement().text().toInt();
                QVariant variant;
                variant.setValue(v);
                _object->setProperty(qPrintable(propName), variant);
            }
            else
            {
                QString v = node.toElement().text();
                _object->setProperty(qPrintable(propName), QVariant(v));
            }
        }
    }
    catch(std::exception& e)
    {
        std::cout<<e.what()<<" - QObjectSerializer::deserialize() "<<std::endl;
        return false;
    }
    return true;
}

