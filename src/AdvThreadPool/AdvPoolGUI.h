// FILE AdvPoolGUI.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef CTHREADPOOLGUI_H
#define CTHREADPOOLGUI_H

#include "CheckCoreWidget.h"
#include <QStandardItemModel>
#include "qtableview.h"
#include <qmainwindow.h>
#include "ServiceStructures.h"
#include "ThreadPoolSettings.h"
#include "WarningJournal.h"

namespace Ui
{
    class ThreadPoolShell;
}

class CAdvPoolGUI:public QMainWindow
{
    Q_OBJECT

private://fields
    Ui::ThreadPoolShell *ui = nullptr;
    eAffinityMode affinityMode = eAffinityMode::No_Affinity;
    int coresQuantity = 0;
    std::list<CheckCoreWidget*> affinityWidgetList;

    WarningJournal* warningJournal = nullptr;

public://methods
    CAdvPoolGUI(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~CAdvPoolGUI();
    void createThreadPoolShell();

private://methods
    void windowClean();
    void setEnabledAffinity(bool sign);

public slots:
    void slot_PoolState(int state);
    void slot_setQuantityUnsharedThreads(int unshared);
    void slot_setQuantitySharedThreads(int shared);
    void slot_NewThread(int id, int type);
    void slot_UnsharedThread_AddLongTask(int id, int runType, QString description);
    void slot_UnsharedThread_DeleteLongTask(int id);
    void slot_UnsharedThread_DeleteExtraLongTask(int id);
    void slot_LongTaskQuantity(int quantity);
    void slot_StopClicked();
    void slot_StartClicked();
    void slot_Apply();
    void slot_DialogHide();
    void slot_SharedThread_MeanCountTasks(int id, QString count);
    void slot_AddRepeatTask(int task_id, int time, QString who, int repeatTaskQuantity);
    void slot_DeleteRepeatTask(int task_id);
    void slot_EditRepeatTaskTime(int task_id, int time);
    void slot_AffinityMode(int index);
    void slot_setQuantityCores(int quantityCores);
    void slot_setAffinityMode(eAffinityMode _affinityMode);
    void slot_setAffinityMask(int thread_num, int thread_type, int thread_mask);
};

#endif // CTHREADPOOLSHELL_H
