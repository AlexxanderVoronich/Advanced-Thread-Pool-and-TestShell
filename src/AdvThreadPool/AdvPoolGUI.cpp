// FILE AdvPoolGUI.cpp
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "AdvPoolGUI.h"
#include <ui_threadpoolshell.h>
#include <AdvThreadPool.h>
#include <QCheckBox>
#include <QTime>

cAdvPoolGUI::cAdvPoolGUI(QWidget* parent, Qt::WindowFlags flags):QMainWindow(parent, flags), m_ui(new Ui::ThreadPoolShell)
{
    m_ui->setupUi(this);

    m_warningJournal = new cWarningJournal(this, QString("Messages box"));
    addDockWidget(Qt::BottomDockWidgetArea, m_warningJournal->m_warningDockWidget);
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_warningJournal->m_warningDockWidget->toggleViewAction());
    m_warningJournal->m_warningDockWidget->hide();

    //this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    m_ui->treeWidget_notshared_threads->header()->resizeSection(0,70);
    m_ui->treeWidget_notshared_threads->header()->resizeSection(1,70);
    m_ui->treeWidget_notshared_threads->header()->resizeSection(2,400);
    m_ui->treeWidget_notshared_threads->header()->stretchLastSection();

    m_ui->treeWidget_shared_threads->header()->resizeSection(0,70);
    m_ui->treeWidget_shared_threads->header()->resizeSection(1,300);
    m_ui->treeWidget_shared_threads->header()->stretchLastSection();

}

cAdvPoolGUI::~cAdvPoolGUI()
{
    delete m_ui;
}

void cAdvPoolGUI::createThreadPoolShell()
{
    cAdvPoolEmitter* emitter = cAdvThreadPool::getInstance().getEmitter();

    connect(emitter, SIGNAL(signal_PoolState(int)), this, SLOT(slot_poolState(int)));
    connect(emitter, SIGNAL(signal_NewThread(int, int)), this, SLOT(slot_newThread(int, int)));
    connect(emitter, SIGNAL(signal_UnsharedThread_AddLongTask(int,int, QString)), this, SLOT(slot_unsharedThread_addLongTask(int,int, QString)));
    connect(emitter, SIGNAL(signal_SharedThread_MeanCountTasks(int, QString)), this, SLOT(slot_sharedThread_meanCountTasks(int, QString)));
    connect(emitter, SIGNAL(signal_UnsharedThread_DeleteLongTask(int)), this, SLOT(slot_unsharedThread_deleteLongTask(int)));
    connect(emitter, SIGNAL(signal_UnsharedThread_DeleteExtraLongTask(int)), this, SLOT(slot_unsharedThread_deleteExtraLongTask(int)));

    connect(emitter, SIGNAL(signal_LongTaskQuantity(int)), this, SLOT(slot_longTaskQuantity(int)));

    connect(emitter, SIGNAL(signal_AddRepeatTask(int, int, QString, int)), this, SLOT(slot_addRepeatTask(int, int, QString, int)));
    connect(emitter, SIGNAL(signal_DeleteRepeatTask(int)), this, SLOT(slot_deleteRepeatTask(int)));
    connect(emitter, SIGNAL(signal_EditRepeatTaskTime(int, int)), this, SLOT(slot_editRepeatTaskTime(int, int)));

    connect(emitter, SIGNAL(signal_QuantityUnsharedThreads(int)), this, SLOT(slot_setQuantityUnsharedThreads(int)));
    connect(emitter, SIGNAL(signal_QuantitySharedThreads(int)), this, SLOT(slot_setQuantitySharedThreads(int)));
    connect(emitter, SIGNAL(signal_QuantityCores(int)), this, SLOT(slot_setQuantityCores(int)));
    connect(emitter, SIGNAL(signal_AffinityMode(eAffinityMode)), this, SLOT(slot_setAffinityMode(eAffinityMode)));
    connect(emitter, SIGNAL(signal_AffinityMask(int, int, int)), this, SLOT(slot_setAffinityMask(int, int, int)));

    connect(m_ui->pushButton_StopPool,SIGNAL(clicked()), this, SLOT(slot_stopClicked()));
    connect(m_ui->pushButton_StartPool,SIGNAL(clicked()), this, SLOT(slot_startClicked()));
    connect(m_ui->pushButton_Apply,SIGNAL(clicked()), this, SLOT(slot_apply()));
    connect(m_ui->pushButton_Close,SIGNAL(clicked()), this, SLOT(slot_dialogHide()));   

    connect(m_ui->comboBox_Affinity,SIGNAL(currentIndexChanged(int)), this, SLOT(slot_affinityMode(int)));
    connect(emitter, SIGNAL(signal_AddWarning(QString, eLogWarning)), m_warningJournal, SLOT(slot_addWarning(QString, eLogWarning)));

    cAdvThreadPool::getInstance().requestState();
}

void cAdvPoolGUI::slot_poolState(int state)
{
    //QPalette palette;
    if(state == 0)
    {
        m_ui->label_state->setText("Power OFF");
        m_ui->label_state->setStyleSheet("QLabel { background-color : red; color : blue; }");
        //palette.setColor(QPalette::ColorRole::Base, Qt::red);
        //ui->label_state->setPalette(palette);
    }
    else
    {
        m_ui->label_state->setText("Power ON");
        m_ui->label_state->setStyleSheet("QLabel { background-color : green; color : yellow; }");
        //palette.setColor(QPalette::ColorRole::Base, Qt::green);
        //ui->label_state->setPalette(palette);
    }
}

void cAdvPoolGUI::slot_setQuantityUnsharedThreads(int unshared)
{
    m_ui->lineEdit_unshared_threads->setText(QString("%1").arg(unshared));
    m_ui->label_UnsharedQ->setText(QString("%1").arg(unshared));
}

void cAdvPoolGUI::slot_setQuantitySharedThreads(int shared)
{
    m_ui->lineEdit_shared_threads->setText(QString("%1").arg(shared));
    m_ui->label_SharedQ->setText(QString("%1").arg(shared));
}


void cAdvPoolGUI::slot_newThread(int id, int type)
{
    QColor colorText = QColor(Qt::black);
    QColor colorShared = QColor(Qt::red);
    QColor colorNotShared = QColor(Qt::green);
    QColor colorCommon = QColor(Qt::yellow);
    QColor colorAffinity = QColor(Qt::gray);
    QTreeWidgetItem* pItem = nullptr;
    if(type == int(cAdvThread::eThreadType::THREAD_NOT_SHARED))
    {
        pItem = new QTreeWidgetItem(m_ui->treeWidget_notshared_threads);
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
    else if(type == int(cAdvThread::eThreadType::THREAD_NOT_SHARED_EXTRA))
    {
        pItem = new QTreeWidgetItem(m_ui->treeWidget_notshared_threads);
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
    else if(type == int(cAdvThread::eThreadType::THREAD_SHARED))
    {
        pItem = new QTreeWidgetItem(m_ui->treeWidget_shared_threads);
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


    cCheckCoreWidget *pWidget = new cCheckCoreWidget(this, m_coresQuantity, id);
    m_affinityWidgetList.push_back(pWidget);

    if(m_affinityMode == eAffinityMode::YES_AFFINITY)
    {
       pWidget->setEnabled(true);
    }
    else if(m_affinityMode == eAffinityMode::NO_AFFINITY ||
            m_affinityMode == eAffinityMode::Yes_Affinity_Without_GUI_Edition ||
            m_affinityMode == eAffinityMode::No_Affinity_Without_GUI_Edition)
    {
       pWidget->setEnabled(false);
    }

    if(type == int(cAdvThread::eThreadType::THREAD_NOT_SHARED) ||
            type == int(cAdvThread::eThreadType::THREAD_NOT_SHARED_EXTRA))
    {
        m_ui->treeWidget_notshared_threads->setItemWidget(pItem, 3, pWidget);
        m_ui->treeWidget_notshared_threads->addTopLevelItem(pItem);
    }
    else if(type == int(cAdvThread::eThreadType::THREAD_SHARED))
    {
        m_ui->treeWidget_shared_threads->setItemWidget(pItem, 2, pWidget);
        m_ui->treeWidget_shared_threads->addTopLevelItem(pItem);
    }
}

void cAdvPoolGUI::slot_unsharedThread_addLongTask(int id, int runType, QString description)
{
    if(runType == int(eRunnableType::LONG_TASK))
    {
        auto list = m_ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
        for(auto it: list)
        {
            it->setText(1, QString("Long task"));
            it->setText(2, description);
        }
    }
    else if(runType == int(eRunnableType::EMPTY_TASK))
    {
        auto list = m_ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
        for(auto it: list)
        {
            it->setText(1, QString("Empty task"));
            it->setText(2, description);
        }
    }
    else if(runType == int(eRunnableType::LONG_TASK_EXTRA))
    {
        auto list = m_ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
        for(auto it: list)
        {
            it->setText(1, QString("Extra task"));
            it->setText(2, description);
        }
    }
}

void cAdvPoolGUI::slot_unsharedThread_deleteLongTask(int id)
{
    auto list = m_ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
        it->setText(1, "Empty task");
        it->setText(2, "empty");
    }
}

void cAdvPoolGUI::slot_unsharedThread_deleteExtraLongTask(int id)
{     
    auto list = m_ui->treeWidget_notshared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
        m_ui->treeWidget_notshared_threads->takeTopLevelItem(m_ui->treeWidget_notshared_threads->indexOfTopLevelItem(it));
    }
}

void cAdvPoolGUI::slot_longTaskQuantity(int quantity)
{
    m_ui->label_longTaskQ->setText(QString::number(quantity));
    qint32 unsharedQ = m_ui->label_UnsharedQ->text().toInt();
    if(quantity > unsharedQ)
    {
        m_ui->label_longTaskQ->setStyleSheet("QLabel { background-color : yellow; color : red; }");
    }
    else
    {
        m_ui->label_longTaskQ->setStyleSheet("");
    }
}

void cAdvPoolGUI::slot_stopClicked()
{
    cAdvThreadPool::stopThreadPool();
    windowClean();
    cAdvThreadPool::getInstance().requestState();
}

void cAdvPoolGUI::slot_startClicked()
{
    cAdvThreadPool::startThreadPool(-1, -1);
    windowClean();
    cAdvThreadPool::getInstance().requestState();
}

void cAdvPoolGUI::slot_apply()
{
    int unshared = m_ui->lineEdit_unshared_threads->text().toInt();
    int shared = m_ui->lineEdit_shared_threads->text().toInt();   
/*
    std::vector<int> affMaskContainer;
    for(auto it: m_AffWidgetList)
    {
        affMaskContainer.push_back(it->getMask());
    }
*/
    cAdvThreadPool::stopThreadPool();
    windowClean();
    cAdvThreadPool::getInstance().saveSettings(unshared, shared, m_affinityMode);
    cAdvThreadPool::startThreadPool(-1, -1);
    cAdvThreadPool::getInstance().requestState();
}


void cAdvPoolGUI::windowClean()
{
    m_ui->treeWidget_notshared_threads->clear();
    m_ui->treeWidget_shared_threads->clear();
    m_affinityWidgetList.clear();
}

void cAdvPoolGUI::slot_dialogHide()
{
    hide();
}

void cAdvPoolGUI::slot_sharedThread_meanCountTasks(int id, QString count)
{
    auto list = m_ui->treeWidget_shared_threads->findItems(QString("%1").arg(id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
        it->setText(1, count);
    }
}

void cAdvPoolGUI::slot_addRepeatTask(int task_id, int time, QString who, int repeatTaskQuantity)
{
    QTreeWidgetItem* pItem;
    pItem = new QTreeWidgetItem(m_ui->treeWidget_repeat_tasks);

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

    m_ui->treeWidget_repeat_tasks->addTopLevelItem(pItem);
    m_ui->label_repeatTaskQ->setText(QString::number(repeatTaskQuantity));
}

void cAdvPoolGUI::slot_deleteRepeatTask(int task_id)
{
    auto list = m_ui->treeWidget_repeat_tasks->findItems(QString("%1").arg(task_id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
        m_ui->treeWidget_repeat_tasks->takeTopLevelItem( m_ui->treeWidget_repeat_tasks->indexOfTopLevelItem(it));
    }
}

void cAdvPoolGUI::slot_editRepeatTaskTime(int task_id, int time)
{
    auto list = m_ui->treeWidget_repeat_tasks->findItems(QString("%1").arg(task_id), Qt::MatchExactly, 0);
    for(auto it: list)
    {
         it->setText(1, QString("%1").arg(time));
    }
}

void cAdvPoolGUI::slot_affinityMode(int _index)
{
    if(_index == 0)
    {
        m_affinityMode = eAffinityMode::NO_AFFINITY;
        setEnabledAffinity(false);
    }
    else if(_index == 1)
    {
        m_affinityMode = eAffinityMode::YES_AFFINITY;
        setEnabledAffinity(true);
    }
}

void cAdvPoolGUI::setEnabledAffinity(bool _sign)
{
    int threadNumber = 0;
    for(auto it: m_affinityWidgetList)
    {
        if(threadNumber != 0)
            it->setEnabled(_sign);
        else
            it->setEnabled(false);//affinityWidget for (zero thread) system thread don't change
        threadNumber++;
    }
}

void cAdvPoolGUI::slot_setQuantityCores(int quantityCores)
{
    m_coresQuantity = quantityCores;
    m_ui->label_CoreQuantity->setText(QString("%1").arg(quantityCores));
}

void cAdvPoolGUI::slot_setAffinityMode(eAffinityMode _affinityMode)
{
    m_affinityMode = eAffinityMode(_affinityMode);
    if(m_affinityMode == eAffinityMode::NO_AFFINITY)
    {
        setEnabledAffinity(false);
        m_ui->comboBox_Affinity->setCurrentIndex(0);
    }
    else if(m_affinityMode == eAffinityMode::YES_AFFINITY)
    {
        setEnabledAffinity(true);
        m_ui->comboBox_Affinity->setCurrentIndex(1);
    }
    else if(m_affinityMode == eAffinityMode::No_Affinity_Without_GUI_Edition)
    {
        setEnabledAffinity(false);
        m_ui->comboBox_Affinity->hide();
        QString title = windowTitle() + QString(" (mode: No_Affinity_Without_GUI_Edition");
        setWindowTitle(title);
    }
    else if(m_affinityMode == eAffinityMode::Yes_Affinity_Without_GUI_Edition)
    {
        setEnabledAffinity(false);
        m_ui->comboBox_Affinity->hide();
        QString title = windowTitle() + QString(" (mode: Yes_Affinity_Without_GUI_Edition");
        setWindowTitle(title);
    }
}

void cAdvPoolGUI::slot_setAffinityMask(int thread_num, int thread_type, int thread_mask)
{
    if(thread_num == 0)
        return; //no affinity for system thread

    int i = 0;
    for(auto it = m_affinityWidgetList.begin(); it != m_affinityWidgetList.end(); it++)
    {
        if(i == thread_num)
        {
            (*it)->setMask(thread_mask);
            break;
        }

        i++;
    }
}
