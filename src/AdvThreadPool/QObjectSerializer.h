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
    static bool serialize(QString _filePath, QObject* _object);
    static bool deserialize(QString _filePath, QObject* _object);
};

#endif // QOBJECTSERIALIZER_H
