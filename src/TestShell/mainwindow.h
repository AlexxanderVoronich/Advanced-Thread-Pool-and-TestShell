#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TaskContainer.h"
#include <QMainWindow>
#include <QStandardItemModel>
#include <QToolBar>
#include <AdvThreadPool/WarningJournal.h>

namespace Ui {
class AdvancedThreadPoolTestShell;
}

class CAdvPoolGUI;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private://fields
    Ui::AdvancedThreadPoolTestShell *ui;
    QToolBar *pToolBar = nullptr;

    QAction* threadPoolAction = nullptr;
    QAction* exitAction = nullptr;

    CAdvPoolGUI* pThreadPoolDialog = nullptr;
    Tasks* pTaskContainer = nullptr;
    ShortTaskGenerator* pShortTaskGenerator = nullptr;

    WarningJournal* warningJournal = nullptr;

public://methods
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private://methods
    void createActions();
    void createConnections();    
    void cleanLongFuturesIfNeed();    
    void closeEvent(QCloseEvent *event);

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
    void signal_AddWarning(QString text, eLogWarning);


};

#endif // MAINWINDOW_H
