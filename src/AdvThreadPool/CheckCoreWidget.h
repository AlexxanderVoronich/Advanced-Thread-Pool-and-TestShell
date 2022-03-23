// FILE CheckCoreWidget.h
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#ifndef CHECKCOREWIDGET_H
#define CHECKCOREWIDGET_H

#include <QSignalMapper>
#include <QtCore/QVariant>
//#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>
#include "qevent.h"

class cCheckCoreWidget:public QWidget
{
    Q_OBJECT

private://fields
    int m_coreQuantity = 0;
    int m_thread_id = 0;
    QGridLayout* m_gridLayout;
    QHBoxLayout* m_horizontalLayout;
    std::vector<QCheckBox*> m_checkBoxContainer;

public://methods
    cCheckCoreWidget(QWidget *_parent, int _core_quantity, int _id);
    ~cCheckCoreWidget(){}
    int getMask() const;
    void setMask(int _mask);

private://methods
    void setupUi(QWidget *_coreWidget); // setupUi
    void mousePressEvent(QMouseEvent *event);
    void setCheckBoxState(int indexCheckBox, bool state);

private slots:
    void slot_CheckBoxClicked(bool _state);

};

#endif // CHECKCOREWIDGET_H
