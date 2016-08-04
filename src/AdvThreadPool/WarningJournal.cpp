#include "WarningJournal.h"
#include <QHeaderView>
#include <QTime>

WarningJournal::WarningJournal(QWidget* parent, QString text)
{
    setupDockWidgets(parent, text);
}

//warning table
void WarningJournal::setupDockWidgets(QWidget* parent, QString text)
{
    warningDockWidget = new CPoolDockWidget(parent);
    warningDockWidget->setObjectName(QString::fromUtf8("m_Warning_dockWidget"));
    warningDockWidget->setWindowTitle(text);
    //m_Warning_dockWidget->windowTitle("Window Imitator Event");

    warningTableView = new QTableView();
    warningTableView->setObjectName(QString::fromUtf8("m_Warning_TV"));

    warningDockWidget->setWidget(warningTableView);

    QStringList headWarn;
    headWarn<<"ico"<<"Time"<<"Message";

    warningModel = new QStandardItemModel(0, headWarn.size() );
    warningModel->setHorizontalHeaderLabels(headWarn);

    warningTableView->setModel(warningModel);
    warningTableView->setSortingEnabled(false);
    warningTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    warningTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    warningTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    warningTableView->setAlternatingRowColors(true);
    //m_Warning_TV->verticalHeader()->hide();
    //m_Warning_TV->horizontalHeader()->setDefaultSectionSize(100);
    //warningTableView->horizontalHeader()->setMovable(false);
    warningTableView->horizontalHeader()->setSectionsMovable(false);
    warningTableView->verticalHeader()->setDefaultSectionSize(16);

    //warning table
    warningTableView->setColumnWidth(0,25);
    warningTableView->setColumnWidth(1,90);
    warningTableView->horizontalHeader()->setStretchLastSection(true);
}

void WarningJournal::slot_AddWarning(QString text, eLogWarning value)
{
    QTime l_oTime = QTime::currentTime();
    QString str_time = l_oTime.toString("hh:mm:ss.zzz");
    QVariant var;

    warningModel->insertRows(0, 1, QModelIndex());

    QModelIndex index = warningModel->index(0, 0, QModelIndex()); //first column
    QIcon ico = getIconForWarning(value);
    warningModel->setData(index, ico , Qt::DecorationRole);

    index = warningModel->index(0, 1, QModelIndex()); //second column
    var.setValue(str_time);
    warningModel->setData(index, var, Qt::EditRole);

    index = warningModel->index(0, 2, QModelIndex());////third column
    var.setValue(text);
    warningModel->setData(index, var, Qt::EditRole);

    int iRowCount = warningModel->rowCount();
    if (iRowCount > 200 )
    {
        warningModel->removeRows(warningModel->index( iRowCount-1 , 0, QModelIndex() ).row(), 1, QModelIndex() );
    }
}

QIcon WarningJournal::getIconForWarning(eLogWarning value)
{
    QIcon ico;
    if(value == eLogWarning::warning)
    {
        ico = QIcon(":/images/warning.xpm");
    }
    else if(value == eLogWarning::message)
    {
        ico = QIcon(":/images/message.xpm");
    }
    else
    {
        ico = QIcon(":/images/message.xpm");
    }
    return ico;
}
