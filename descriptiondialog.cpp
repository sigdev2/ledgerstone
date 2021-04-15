#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "descriptiondialog.h"

DescriptionDialog::DescriptionDialog(const QString& desc, QWidget* parent) : QDialog(parent)
{
    QVBoxLayout* main = new QVBoxLayout(this);

    m_desc = new QTextEdit(this);
    m_desc->setText(desc);
    m_desc->setMinimumSize(200, 100);

    main->addWidget(m_desc);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    main->addLayout(buttonsLayout);

    QPushButton* ok = new QPushButton(tr("OK"), this);
    buttonsLayout->addWidget(ok);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton* cancel = new QPushButton(tr("Cancel"), this);
    buttonsLayout->addWidget(cancel);
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
}
