#include "WarningJournal.h"
#include <QHeaderView>
#include <QTime>

cWarningJournal::cWarningJournal(QWidget* parent, QString text)
{
    setupDockWidgets(parent, text);
}

//warning table
void cWarningJournal::setupDockWidgets(QWidget* parent, QString text)
{
    m_warningDockWidget = new cPoolDockWidget(parent);
    m_warningDockWidget->setObjectName(QString::fromUtf8("m_Warning_dockWidget"));
    m_warningDockWidget->setWindowTitle(text);
    //m_Warning_dockWidget->windowTitle("Window Imitator Event");

    m_warningTableView = new QTableView();
    m_warningTableView->setObjectName(QString::fromUtf8("m_Warning_TV"));

    m_warningDockWidget->setWidget(m_warningTableView);

    QStringList headWarn;
    headWarn<<"ico"<<"Time"<<"Message";

    m_warningModel = new QStandardItemModel(0, headWarn.size() );
    m_warningModel->setHorizontalHeaderLabels(headWarn);

    m_warningTableView->setModel(m_warningModel);
    m_warningTableView->setSortingEnabled(false);
    m_warningTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_warningTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_warningTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_warningTableView->setAlternatingRowColors(true);
    //m_Warning_TV->verticalHeader()->hide();
    //m_Warning_TV->horizontalHeader()->setDefaultSectionSize(100);
    //warningTableView->horizontalHeader()->setMovable(false);
    m_warningTableView->horizontalHeader()->setSectionsMovable(false);
    m_warningTableView->verticalHeader()->setDefaultSectionSize(16);

    //warning table
    m_warningTableView->setColumnWidth(0,25);
    m_warningTableView->setColumnWidth(1,90);
    m_warningTableView->horizontalHeader()->setStretchLastSection(true);
}

void cWarningJournal::slot_addWarning(QString text, eLogWarning value)
{
    QTime l_oTime = QTime::currentTime();
    QString str_time = l_oTime.toString("hh:mm:ss.zzz");
    QVariant var;

    m_warningModel->insertRows(0, 1, QModelIndex());

    QModelIndex index = m_warningModel->index(0, 0, QModelIndex()); //first column
    QIcon ico = getIconForWarning(value);
    m_warningModel->setData(index, ico , Qt::DecorationRole);

    index = m_warningModel->index(0, 1, QModelIndex()); //second column
    var.setValue(str_time);
    m_warningModel->setData(index, var, Qt::EditRole);

    index = m_warningModel->index(0, 2, QModelIndex());////third column
    var.setValue(text);
    m_warningModel->setData(index, var, Qt::EditRole);

    int iRowCount = m_warningModel->rowCount();
    if (iRowCount > 200 )
    {
        m_warningModel->removeRows(m_warningModel->index( iRowCount-1 , 0, QModelIndex() ).row(), 1, QModelIndex() );
    }
}

QIcon cWarningJournal::getIconForWarning(eLogWarning value)
{
    QIcon ico;
    if(value == eLogWarning::WARNING)
    {
        ico = QIcon(":/images/warning.xpm");
    }
    else if(value == eLogWarning::MESSAGE)
    {
        ico = QIcon(":/images/message.xpm");
    }
    else
    {
        ico = QIcon(":/images/message.xpm");
    }
    return ico;
}
