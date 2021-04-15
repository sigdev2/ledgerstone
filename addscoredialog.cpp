#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

#include "database.h"

#include "addscoredialog.h"

AddScoreDialog::AddScoreDialog(QWidget* parent) : QDialog(parent)
{
    QVBoxLayout* main = new QVBoxLayout(this);

    m_name = new QLineEdit(this);

    main->addWidget(m_name);

    QButtonGroup* radioGroup = new QButtonGroup(this);

    m_none = new QRadioButton(tr("None"), this);
    radioGroup->addButton(m_none);
    main->addWidget(m_none);

    m_active = new QRadioButton(tr("Active"), this);
    radioGroup->addButton(m_active);
    main->addWidget(m_active);

    m_passive = new QRadioButton(tr("Passive"), this);
    radioGroup->addButton(m_passive);
    main->addWidget(m_passive);

    m_income = new QRadioButton(tr("Incomings"), this);
    radioGroup->addButton(m_income);
    main->addWidget(m_income);

    m_cost = new QRadioButton(tr("Spendings"), this);
    m_cost->setChecked(true);
    radioGroup->addButton(m_cost);
    main->addWidget(m_cost);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    main->addLayout(buttonsLayout);

    QPushButton* ok = new QPushButton(tr("OK"), this);
    buttonsLayout->addWidget(ok);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton* cancel = new QPushButton(tr("Cancel"), this);
    buttonsLayout->addWidget(cancel);
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

quint64 AddScoreDialog::flags() const
{
    quint64 flags = DataBase::eNone;
    if (m_none->isChecked())
        return flags;
    if (m_active->isChecked())
        flags |= DataBase::eActive;
    if (m_passive->isChecked())
        flags |= DataBase::ePassive;
    if (m_income->isChecked())
        flags |= DataBase::eIncoming;
    if (m_cost->isChecked())
        flags |= DataBase::eSpending;
    return flags;
}
