// FILE AdvPoolGUI.cpp
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "AdvPoolGUI.h"
#include <ui_threadpoolshell.h>
#include <AdvThreadPool.h>
#include <QCheckBox>
#include <QTime>

CAdvPoolGUI::CAdvPoolGUI(QWidget* parent, Qt::WindowFlags flags):QMainWindow(parent, flags), ui(new Ui::ThreadPoolShell)
{
    ui->setupUi(this);

    warningJournal = new WarningJournal(this, QString("Messages box"));
    addDockWidget(Qt::BottomDockWidgetArea, warningJournal->warningDockWidget);
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(warningJournal->warningDockWidget->toggleViewAction());
    warningJournal->warningDockWidget->hide();

    //this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    ui->treeWidget_notshared_threads->header()->resizeSection(0,70);
    ui->treeWidget_notshared_threads->header()->resizeSection(1,70);
    ui->treeWidget_notshared_threads->header()->resizeSection(2,400);
    ui->treeWidget_notshared_threads->header()->stretchLastSection();

    ui->treeWidget_shared_threads->header()->resizeSection(0,70);
    ui->treeWidget_shared_threads->header()->resizeSection(1,300);
    ui->treeWidget_shared_threads->header()->stretchLastSection();

}

CAdvPoolGUI::~CAdvPoolGUI()
{
    delete ui;
}

void CAdvPoolGUI::createThreadPoolShell()
{
    CAdvPoolEmitter* emitter = CAdvThreadPool::getInstance().getEmitter();

    connect(emitter, SIGNAL(signal_PoolState(int)), this, SLOT(slot_PoolState(int)));
    connect(emitter, SIGNAL(signal_NewThread(int, int)), this, SLOT(slot_NewThread(int, int)));
    connect(emitter, SIGNAL(signal_UnsharedThread_AddLongTask(int,int, QString)), this, SLOT(slot_UnsharedThread_AddLongTask(int,int, QString)));
    connect(emitter, SIGNAL(signal_SharedThread_MeanCountTasks(int, QString)), this, SLOT(slot_SharedThread_MeanCountTasks(int, QString)));
    connect(emitter, SIGNAL(signal_UnsharedThread_DeleteLongTask(int)), this, SLOT(slot_UnsharedThread_DeleteLongTask(int)));
    connect(emitter, SIGNAL(signal_UnsharedThread_DeleteExtraLongTask(int)), this, SLOT(slot_UnsharedThread_DeleteExtraLongTask(int)));

    connect(emitter, SIGNAL(signal_LongTaskQuantity(int)), this, SLOT(slot_LongTaskQuantity(int)));

    connect(emitter, SIGNAL(signal_AddRepeatTask(int, int, QString, int)), this, SLOT(slot_AddRepeatTask(int, int, QString, int)));
    connect(emitter, SIGNAL(signal_DeleteRepeatTask(int)), this, SLOT(slot_DeleteRepeatTask(int)));
    connect(emitter, SIGNAL(signal_EditRepeatTaskTime(int, int)), this, SLOT(slot_EditRepeatTaskTime(int, int)));

    connect(emitter, SIGNAL(signal_QuantityUnsharedThreads(int)), this, SLOT(slot_setQuantityUnsharedThreads(int)));
    connect(emitter, SIGNAL(signal_QuantitySharedThreads(int)), this, SLOT(slot_setQuantitySharedThreads(int)));
    connect(emitter, SIGNAL(signal_QuantityCores(int)), this, SLOT(slot_setQuantityCores(int)));
    connect(emitter, SIGNAL(signal_AffinityMode(eAffinityMode)), this, SLOT(slot_setAffinityMode(eAffinityMode)));
    connect(emitter, SIGNAL(signal_AffinityMask(int, int, int)), this, SLOT(slot_setAffinityMask(int, int, int)));

    connect(ui->pushButton_StopPool,SIGNAL(clicked()), this, SLOT(slot_StopClicked()));
    connect(ui->pushButton_StartPool,SIGNAL(clicked()), this, SLOT(slot_StartClicked()));
    connect(ui->pushButton_Apply,SIGNAL(clicked()), this, SLOT(slot_Apply()));
    connect(ui->pushButton_Close,SIGNAL(clicked()), this, SLOT(slot_DialogHide()));   

    connect(ui->comboBox_Affinity,SIGNAL(currentIndexChanged(int)), this, SLOT(slot_AffinityMode(int)));
    connect(emitter, SIGNAL(signal_AddWarning(QString, eLogWarning)), warningJournal, SLOT(slot_AddWarning(QString, eLogWarning)));

    CAdvThreadPool::getInstance().requestState();
}

void CAdvPoolGUI::slot_PoolState(int state)
{
    //QPalette palette;
    if(state == 0)
    {
        ui->label_state->setText("Power OFF");
        ui->label_state->setStyleSheet("QLabel { background-color : red; color : blue; }");
        //palette.setColor(QPalette::ColorRole::Base, Qt::red);
        //ui->label_state->setPalette(palette);
    }
    else
    {
        ui->label_state->setText("Power ON");
        ui->label_state->setStyleSheet("QLabel { background-color : green; color : yellow; }");
        //palette.setColor(QPalette::ColorRole::Base, Qt::green);
        //ui->label_state->setPalette(palette);
    }
}

void CAdvPoolGUI::slot_setQuantityUnsharedThreads(int unshared)
{
    ui->lineEdit_unshared_threads->setText(QString("%1").arg(unshared));
    ui->label_UnsharedQ->setText(QString("%1").arg(unshared));
}

void CAdvPoolGUI::slot_setQuantitySharedThreads(int shared)
{
    ui->lineEdit_shared_threads->setText(QString("%1").arg(shared));
    ui->label_SharedQ->setText(QString("%1").arg(shared));
}


void CAdvPoolGUI::slot_NewThread(int id, int type)
{
    QColor colorText = QColor(Qt::black);
    QColor colorShared = QColor(Qt::red);
    QColor colorNotShared = QColor(Qt::green);
    QColor colorCommon = QColor(Qt::yellow);
    QColor colorAffinity = QColor(Qt::gray);
    QTreeWidgetItem* pItem = nullptr;
    if(type == int(CAdvThread::eThreadType::THREAD_NOT_SHARED))
    {
        pItem = new QTreeWidgetItem(ui->treeWidget_notshared_threads);
        pItem->setBackground(0, QBrush(colorNotShared));
        pItem->setForeground(0, QBrush(colorText));
        pItem->setBackground(1, QBrush(colorCommon));
        pItem->setForeground(1, QBrush(colorText));
        pItem->setBackground(2, QBrush(colorCommon));
        pItem->setForeground(2, QBrush(colorText));
        pItem->setBackground(3, QBrush(colorAffinity));
        pItem->setForeground(3, QBrush(colorText));

        pItem->setText(0, QString("%1").arg(id));
    }
    else if(type == int(CAdvThread::eThreadType::THREAD_NOT_SHARED_EXTRA))
    {
        pItem = new QTreeWidgetItem(ui->treeWidget_notshared_threads);
        pItem->setBackground(0, QBrush(colorShared));
        pItem->setForeground(0, QBrush(colorText));
        pItem->setBackground(1, QBrush(colorCommon));
        pItem->setForeground(1, QBrush(colorText));
        pItem->setBackground(2, QBrush(colorCommon));
        pItem->setForeground(2, QBrush(colorText));
        pItem->setBackground(3, QBrush(colorAffinity));
        pItem->setForeground(3, QBrush(colorText));

        pItem->setText(0, QString("%1").arg(id));
    }
    else if(type == int(CAdvThread::eThreadType::THREAD_SHARED))
    {
        pItem = new QTreeWidgetItem(ui->treeWidget_shared_threads);
        pItem->setBackground(0, QBrush(colorShared));
        pItem->setForeground(0, QBrush(colorText));
        pItem->setBackground(1, QBrush(colorCommon));
        pItem->setForeground(1, QBrush(colorText));
        pItem->setBackground(2, QBrush(colorAffinity));
        pItem->setForeground(2, QBrush(colorText));

        pItem->setText(0, QString("%1").arg(id));
    }
    else
        return;


    CheckCoreWidget *pWidget = new CheckCoreWidget(this, coresQuantity, id);
    affinityWidgetList.push_back(pWidget);

    if(affinityMode == eAffinityMode::Yes_Affinity)
    {
       pWidget->setEnabled(true);
    }
    else if(affinityMode == eAffinityMode::No_Affinity ||
            affinityMode == eAffinityMode::Yes_Affinity_Without_GUI_Edition ||
            affinityMode == eAffinityMode::No_Affinity_Without_GUI_Edition)
    {
       pWidget->setEnabled(false);
    }

    if(type == int(CAdvThread::eThreadType::THREAD_NOT_SHARED) ||
            type == int(CAdvThread::eThreadType::THREAD_NOT_SHARED_EXTRA))
    {
        ui->treeWidget_notshared_threads->setItemWidget(pItem, 3, pWidget);
        ui->treeWidget_notshared_threads->addTopLevelItem(pItem);
    }
    else if(type == int(CAdvThread::eThreadType::THREAD_SHARED))
    {
        ui->treeWidget_shared_threads->setItemWidget(pItem, 2, pWidget);
        ui->treeWidget_shared_threads->addTopLevelItem(pItem);
    }
}

void CAdvPoolGUI::slot_UnsharedThread_AddLongTask(int id, int runType, QString description)
{
    if(runType == int(eRunnableType::LONG_TASK))
    {
        auto list = ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
        for(auto it: list)
        {
            it->setText(1, QString("Long task"));
            it->setText(2, description);
        }
    }
    else if(runType == int(eRunnableType::EMPTY_TASK))
    {
        auto list = ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
        for(auto it: list)
        {
            it->setText(1, QString("Empty task"));
            it->setText(2, description);
        }
    }
    else if(runType == int(eRunnableType::LONG_TASK_EXTRA))
    {
        auto list = ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
        for(auto it: list)
        {
            it->setText(1, QString("Extra task"));
            it->setText(2, description);
        }
    }
}

void CAdvPoolGUI::slot_UnsharedThread_DeleteLongTask(int id)
{
    auto list = ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
        it->setText(1, "Empty task");
        it->setText(2, "empty");
    }
}

void CAdvPoolGUI::slot_UnsharedThread_DeleteExtraLongTask(int id)
{     
    auto list = ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
        ui->treeWidget_notshared_threads->takeTopLevelItem(ui->treeWidget_notshared_threads->indexOfTopLevelItem(it));
    }
}

void CAdvPoolGUI::slot_LongTaskQuantity(int quantity)
{
    ui->label_longTaskQ->setText(QString::number(quantity));
    qint32 unsharedQ = ui->label_UnsharedQ->text().toInt();
    if(quantity > unsharedQ)
    {
        ui->label_longTaskQ->setStyleSheet("QLabel { background-color : yellow; color : red; }");
    }
    else
    {
        ui->label_longTaskQ->setStyleSheet("");
    }
}

void CAdvPoolGUI::slot_StopClicked()
{
    CAdvThreadPool::stopThreadPool();
    windowClean();
    CAdvThreadPool::getInstance().requestState();
}

void CAdvPoolGUI::slot_StartClicked()
{
    CAdvThreadPool::startThreadPool(-1, -1);
    windowClean();
    CAdvThreadPool::getInstance().requestState();
}

void CAdvPoolGUI::slot_Apply()
{
    int unshared = ui->lineEdit_unshared_threads->text().toInt();
    int shared = ui->lineEdit_shared_threads->text().toInt();   
/*
    std::vector<int> affMaskContainer;
    for(auto it: m_AffWidgetList)
    {
        affMaskContainer.push_back(it->getMask());
    }
*/
    CAdvThreadPool::stopThreadPool();
    windowClean();
    CAdvThreadPool::getInstance().saveSettings(unshared, shared, affinityMode);
    CAdvThreadPool::startThreadPool(-1, -1);
    CAdvThreadPool::getInstance().requestState();
}


void CAdvPoolGUI::windowClean()
{
    ui->treeWidget_notshared_threads->clear();
    ui->treeWidget_shared_threads->clear();
    affinityWidgetList.clear();
}

void CAdvPoolGUI::slot_DialogHide()
{
    hide();
}

void CAdvPoolGUI::slot_SharedThread_MeanCountTasks(int id, QString count)
{
    auto list = ui->treeWidget_shared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
        it->setText(1, count);
    }
}

void CAdvPoolGUI::slot_AddRepeatTask(int task_id, int time, QString who, int repeatTaskQuantity)
{
    QTreeWidgetItem* pItem;
    pItem = new QTreeWidgetItem(ui->treeWidget_repeat_tasks);

    QColor color_text = QColor(Qt::black);
    QColor color_id = QColor(135, 206, 250);
    QColor color_common = QColor(Qt::yellow);

    pItem->setBackground(0, QBrush(color_id));
    pItem->setForeground(0, QBrush(color_text));
    pItem->setBackground(1, QBrush(color_common));
    pItem->setForeground(1, QBrush(color_text));
    pItem->setBackground(2, QBrush(color_common));
    pItem->setForeground(2, QBrush(color_text));
    pItem->setText(0, QString("%1").arg(task_id));
    pItem->setText(1, QString("%1").arg(time));
    pItem->setText(2, who);

    ui->treeWidget_repeat_tasks->addTopLevelItem(pItem);
    ui->label_repeatTaskQ->setText(QString::number(repeatTaskQuantity));
}

void CAdvPoolGUI::slot_DeleteRepeatTask(int task_id)
{
    auto list = ui->treeWidget_repeat_tasks->findItems(QString("%1").arg(task_id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
        ui->treeWidget_repeat_tasks->takeTopLevelItem( ui->treeWidget_repeat_tasks->indexOfTopLevelItem(it));
    }
}

void CAdvPoolGUI::slot_EditRepeatTaskTime(int task_id, int time)
{
    auto list = ui->treeWidget_repeat_tasks->findItems(QString("%1").arg(task_id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
         it->setText(1, QString("%1").arg(time));
    }
}

void CAdvPoolGUI::slot_AffinityMode(int index)
{
    if(index == 0)
    {
        affinityMode = eAffinityMode::No_Affinity;
        setEnabledAffinity(false);
    }
    else if(index == 1)
    {
        affinityMode = eAffinityMode::Yes_Affinity;
        setEnabledAffinity(true);
    }
}

void CAdvPoolGUI::setEnabledAffinity(bool sign)
{
    int threadNumber = 0;
    for(auto it: affinityWidgetList)
    {
        if(threadNumber != 0)
            it->setEnabled(sign);
        else
            it->setEnabled(false);//affinityWidget for (zero thread) system thread don't change
        threadNumber++;
    }
}

void CAdvPoolGUI::slot_setQuantityCores(int quantityCores)
{
    coresQuantity = quantityCores;
    ui->label_CoreQuantity->setText(QString("%1").arg(quantityCores));
}

void CAdvPoolGUI::slot_setAffinityMode(eAffinityMode _affinityMode)
{
    affinityMode = eAffinityMode(_affinityMode);
    if(affinityMode == eAffinityMode::No_Affinity)
    {
        setEnabledAffinity(false);
        ui->comboBox_Affinity->setCurrentIndex(0);
    }
    else if(affinityMode == eAffinityMode::Yes_Affinity)
    {
        setEnabledAffinity(true);
        ui->comboBox_Affinity->setCurrentIndex(1);
    }
    else if(affinityMode == eAffinityMode::No_Affinity_Without_GUI_Edition)
    {
        setEnabledAffinity(false);
        ui->comboBox_Affinity->hide();
        QString title = windowTitle() + QString(" (mode: No_Affinity_Without_GUI_Edition");
        setWindowTitle(title);
    }
    else if(affinityMode == eAffinityMode::Yes_Affinity_Without_GUI_Edition)
    {
        setEnabledAffinity(false);
        ui->comboBox_Affinity->hide();
        QString title = windowTitle() + QString(" (mode: Yes_Affinity_Without_GUI_Edition");
        setWindowTitle(title);
    }
}

void CAdvPoolGUI::slot_setAffinityMask(int thread_num, int thread_type, int thread_mask)
{
    if(thread_num == 0)
        return; //no affinity for system thread

    int i = 0;
    for(auto it = affinityWidgetList.begin(); it != affinityWidgetList.end(); it++)
    {
        if(i == thread_num)
        {
            (*it)->setMask(thread_mask);
            break;
        }

        i++;
    }
}
