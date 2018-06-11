#ifndef WARNINGJOURNAL_H
#define WARNINGJOURNAL_H

#include <QStandardItemModel>
#include <QTableView>
#include "ServiceStructures.h"


class cWarningJournal: public QObject
{
    Q_OBJECT
public:
    cWarningJournal(QWidget* _parent, QString _text);
    void setupDockWidgets(QWidget* _parent, QString _text);
    QIcon getIconForWarning(eLogWarning _value);

    QStandardItemModel* m_warningModel = nullptr;//model of table WarningTable
    QTableView* m_warningTableView = nullptr;//warning Table  View
    cPoolDockWidget *m_warningDockWidget = nullptr;//QDockWidget

private slots:
    void slot_addWarning(QString _text, eLogWarning _value);

};

#endif // WARNINGJOURNAL_H
