#ifndef QOBJECTSERIALIZER_H
#define QOBJECTSERIALIZER_H

#include <QObject>
#include <QDomDocument>
#include <QFile>
#include <QMetaProperty>
#include <QTextStream>
#include <iostream>

Q_DECLARE_METATYPE(std::vector<int>)
class QObjectSerializer
{
public:
    QObjectSerializer();
    static bool serialize(QString filePath, QObject* pObject);
    static bool deserialize(QString filePath, QObject* pObject);
};

#endif // QOBJECTSERIALIZER_H
