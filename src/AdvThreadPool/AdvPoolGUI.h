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

class IMPORT_EXPORT cAdvPoolGUI:public QMainWindow
{
    Q_OBJECT

private://fields
    Ui::ThreadPoolShell *m_ui = nullptr;
    eAffinityMode m_affinityMode = eAffinityMode::NO_AFFINITY;
    int m_coresQuantity = 0;
    std::list<cCheckCoreWidget*> m_affinityWidgetList;

    cWarningJournal* m_warningJournal = nullptr;

public://methods
    cAdvPoolGUI(QWidget* _parent = 0, Qt::WindowFlags _flags = Qt::Widget);
    ~cAdvPoolGUI();
    void createThreadPoolShell();

private://methods
    void windowClean();
    void setEnabledAffinity(bool _sign);

public slots:
    void slot_poolState(int _state);
    void slot_setQuantityUnsharedThreads(int _unshared);
    void slot_setQuantitySharedThreads(int _shared);
    void slot_newThread(int _id, int _type);
    void slot_unsharedThread_addLongTask(int _id, int _runType, QString _description);
    void slot_unsharedThread_deleteLongTask(int _id);
    void slot_unsharedThread_deleteExtraLongTask(int _id);
    void slot_longTaskQuantity(int _quantity);
    void slot_stopClicked();
    void slot_startClicked();
    void slot_apply();
    void slot_dialogHide();
    void slot_sharedThread_averageQuantityOfTasks(int _id, QString _count);
    void slot_addRepeatTask(int _taskId, int _time, QString _who, int _repeatTaskQuantity);
    void slot_deleteRepeatTask(int _task_id);
    void slot_editRepeatTaskTime(int _taskId, int _time);
    void slot_affinityMode(int _index);
    void slot_setQuantityCores(int _quantityCores);
    void slot_setAffinityMode(eAffinityMode _affinityMode);
    void slot_setAffinityMask(int _threadNum, int _threadType, int _threadMask);
};

#endif // CTHREADPOOLSHELL_H
