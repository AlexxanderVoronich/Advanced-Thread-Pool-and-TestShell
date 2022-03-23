#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TaskContainer.h"
#include <QMainWindow>
#include <QStandardItemModel>
#include <QToolBar>

namespace Ui 
{
    class AdvancedThreadPoolTestShell;
}

class cAdvPoolGUI;
class cWarningJournal;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private://fields
    Ui::AdvancedThreadPoolTestShell *m_ui = nullptr;
    QToolBar *m_toolBar = nullptr;

    QAction* m_threadPoolAction = nullptr;
    QAction* m_exitAction = nullptr;

    cAdvPoolGUI* m_threadPoolDialog = nullptr;
    Tasks* m_taskContainer = nullptr;
    ShortTaskGenerator* m_shortTaskGenerator = nullptr;
    cWarningJournal* m_warningJournal = nullptr;

public://methods
    explicit MainWindow(QWidget *_parent = 0);
    ~MainWindow();

private://methods
    void createActions();
    void createConnections();    
    void cleanLongFuturesIfNeed();    
    void closeEvent(QCloseEvent *_event);

private slots:
    void slot_OnThreadPool();
    void slot_OnExit();
    void slot_CreateLongTask();
    void slot_CreateRepeatTask();
    void slot_ShortTaskGeneratorLaunch();
    void slot_StopLongTask();
    void slot_StopRepeatTask();
    void slot_StopShortTaskGenerator();

signals:
    void signal_AddWarning(QString _text, eLogWarning);
};

#endif // MAINWINDOW_H
