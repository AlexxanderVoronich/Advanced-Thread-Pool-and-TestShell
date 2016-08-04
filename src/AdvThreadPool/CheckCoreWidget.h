// FILE CheckCoreWidget.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef CHECKCOREWIDGET_H
#define CHECKCOREWIDGET_H

#include <QSignalMapper>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>
#include "qevent.h"

class CheckCoreWidget:public QWidget
{
    Q_OBJECT

private://fields
    int coreQuantity = 0;
    int thread_id = 0;
    QGridLayout* gridLayout;
    QHBoxLayout* horizontalLayout;
    std::vector<QCheckBox*> checkBoxContainer;

public://methods
    CheckCoreWidget(QWidget *parent, int core_quantity, int id);
    ~CheckCoreWidget(){}
    int getMask() const;
    void setMask(int mask);

private://methods
    void setupUi(QWidget *CheckCoreWidget); // setupUi
    void mousePressEvent(QMouseEvent *event);
    void setCheckBoxState(int indexCheckBox, bool state);

private slots:
    void slot_CheckBoxClicked(bool state);

};

#endif // CHECKCOREWIDGET_H
