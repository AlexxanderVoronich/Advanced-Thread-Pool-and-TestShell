// FILE CheckCoreWidget.cpp
//
// AUTHOR: Ariman, 12 jan 2015
//////////////////////////////////////////////////////////////////////

#include "AdvThreadPool.h"
#include "CheckCoreWidget.h"


cCheckCoreWidget::cCheckCoreWidget(QWidget *_parent, int _core_quantity, int _id):QWidget(_parent)
{
    m_coreQuantity = _core_quantity;
    m_thread_id = _id;
    setupUi(this);

    for(int i=0; i < m_coreQuantity; i++)
    {
        connect(m_checkBoxContainer[i], SIGNAL(toggled(bool)), this, SLOT(slot_CheckBoxClicked(bool)));
    }
}


void cCheckCoreWidget::mousePressEvent (QMouseEvent * event)
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

        bool state = m_checkBoxContainer[0]->checkState() == Qt::Unchecked ?true:false;

        for(auto it = m_checkBoxContainer.begin(); it != m_checkBoxContainer.end(); it++)
        {
            if(state)
                (*it)->setCheckState(Qt::CheckState::Checked);
            else
                (*it)->setCheckState(Qt::CheckState::Unchecked);
        }

    }

}

int cCheckCoreWidget::getMask() const
{
    int num_core = 0;
    int mask = 0;
    for(auto it: m_checkBoxContainer)
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

void cCheckCoreWidget::setMask(int _mask)
{
    int num_core = 0;
    for(auto it: m_checkBoxContainer)
    {
        int shift = 1<<num_core;
        int temp = _mask & shift;
        if(temp == shift)
        {
            it->setCheckState(Qt::CheckState::Checked);
        }
        num_core++;
    }
}

void cCheckCoreWidget::setCheckBoxState(int indexCheckBox, bool state)
{
    if(state)
        m_checkBoxContainer[indexCheckBox]->setCheckState(Qt::CheckState::Checked);
    else
        m_checkBoxContainer[indexCheckBox]->setCheckState(Qt::CheckState::Unchecked);
}

void cCheckCoreWidget::slot_CheckBoxClicked(bool _state)
{
    QObject* obj = QObject::sender();

    int core_id = 1;
    for(auto it: m_checkBoxContainer)
    {
        if(it == qobject_cast<QCheckBox*>(obj))
        {
            cAdvThreadPool::getInstance().changeAffinityMask(m_thread_id, core_id, _state);
            break;
        }
        core_id++;
    }
}

void cCheckCoreWidget::setupUi(QWidget *_coreWidget)
{
    _coreWidget->setWindowTitle(QApplication::translate("CheckCoreWidget", "Form", 0));

    if (_coreWidget->objectName().isEmpty())
        _coreWidget->setObjectName(QStringLiteral("CheckCoreWidget"));
    _coreWidget->resize(294, 37);

    m_gridLayout = new QGridLayout(_coreWidget);
    m_gridLayout->setObjectName(QStringLiteral("gridLayout"));
    m_horizontalLayout = new QHBoxLayout();
    m_horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));

    for(int i=0; i < m_coreQuantity; i++)
    {
        m_checkBoxContainer.push_back(new QCheckBox(_coreWidget));
    }

    for(int i=0; i < m_coreQuantity; i++)
    {
        m_checkBoxContainer[i]->setObjectName(QString("checkBox_%1").arg(i+1));
        m_horizontalLayout->addWidget(m_checkBoxContainer[i]);
        m_checkBoxContainer[i]->setText(QString("%1").arg(i+1));
    }

    m_gridLayout->addLayout(m_horizontalLayout, 0, 0, 1, 1);
}

