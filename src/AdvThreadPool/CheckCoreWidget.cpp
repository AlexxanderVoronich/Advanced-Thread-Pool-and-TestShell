// FILE CheckCoreWidget.cpp
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "AdvThreadPool.h"
#include "CheckCoreWidget.h"


CheckCoreWidget::CheckCoreWidget(QWidget *parent, int core_quantity, int id):QWidget(parent)
{
    coreQuantity = core_quantity;
    thread_id = id;
    setupUi(this);

    for(int i=0; i<coreQuantity; i++)
    {
        connect(checkBoxContainer[i], SIGNAL(toggled(bool)), this, SLOT(slot_CheckBoxClicked(bool)));
    }
}


void CheckCoreWidget::mousePressEvent (QMouseEvent * event)
{
/*
    if(Qt::LeftButton == event->button())
    {
        QWidget::mousePressEvent(event);
    }
    else if(Qt::MidButton == event->button())
    {
        QWidget::mousePressEvent(event);

    }
    else*/
    if(Qt::RightButton == event->button())
    {
        QWidget::mousePressEvent(event);

        bool state = checkBoxContainer[0]->checkState() == Qt::Unchecked ?true:false;

        for(auto it = checkBoxContainer.begin(); it != checkBoxContainer.end(); it++)
        {
            if(state)
                (*it)->setCheckState(Qt::CheckState::Checked);
            else
                (*it)->setCheckState(Qt::CheckState::Unchecked);
        }

    }

}

int CheckCoreWidget::getMask() const
{
    int num_core = 0;
    int mask = 0;
    for(auto it: checkBoxContainer)
    {
        int chek = it->checkState();
        if(chek == Qt::Unchecked)
        {
        }
        else if(chek == Qt::Checked)
        {
             mask = mask|(1<<num_core);
        }
        else
        {
            return 0;
        }
        num_core++;
    }

    return mask;
}

void CheckCoreWidget::setMask(int mask)
{
    int num_core = 0;
    for(auto it: checkBoxContainer)
    {
        int shift = 1<<num_core;
        int temp = mask & shift;
        if(temp == shift)
        {
            it->setCheckState(Qt::CheckState::Checked);
        }
        num_core++;
    }
}

void CheckCoreWidget::setCheckBoxState(int indexCheckBox, bool state)
{
    if(state)
        checkBoxContainer[indexCheckBox]->setCheckState(Qt::CheckState::Checked);
    else
        checkBoxContainer[indexCheckBox]->setCheckState(Qt::CheckState::Unchecked);
}

void CheckCoreWidget::slot_CheckBoxClicked(bool state)
{
    QObject* obj = QObject::sender();

    int core_id = 1;
    for(auto it: checkBoxContainer)
    {
        if(it == qobject_cast<QCheckBox*>(obj))
        {
            CAdvThreadPool::getInstance().changeAffinityMask(thread_id, core_id, state);
            break;
        }
        core_id++;
    }
}

void CheckCoreWidget::setupUi(QWidget *CheckCoreWidget)
{
    CheckCoreWidget->setWindowTitle(QApplication::translate("CheckCoreWidget", "Form", 0));

    if (CheckCoreWidget->objectName().isEmpty())
        CheckCoreWidget->setObjectName(QStringLiteral("CheckCoreWidget"));
    CheckCoreWidget->resize(294, 37);

    gridLayout = new QGridLayout(CheckCoreWidget);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));

    for(int i=0; i<coreQuantity; i++)
    {
        checkBoxContainer.push_back(new QCheckBox(CheckCoreWidget));
    }

    for(int i=0; i<coreQuantity; i++)
    {
        checkBoxContainer[i]->setObjectName(QString("checkBox_%1").arg(i+1));
        horizontalLayout->addWidget(checkBoxContainer[i]);
        checkBoxContainer[i]->setText(QString("%1").arg(i+1));
    }

    gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);
}

