#ifndef WARNINGJOURNAL_H
#define WARNINGJOURNAL_H

#include <QStandardItemModel>
#include <QTableView>
#include "ServiceStructures.h"


class WarningJournal: public QObject
{
    Q_OBJECT
public:
    WarningJournal(QWidget* parent, QString text);
    void setupDockWidgets(QWidget* parent, QString text);
    QIcon getIconForWarning(eLogWarning value);

    QStandardItemModel* warningModel = nullptr;//model of table WarningTable
    QTableView* warningTableView = nullptr;//warning Table  View
    CPoolDockWidget *warningDockWidget = nullptr;//QDockWidget

private slots:
    void slot_AddWarning(QString text, eLogWarning value);

};

#endif // WARNINGJOURNAL_H
