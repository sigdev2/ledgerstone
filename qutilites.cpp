#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "qutilites.h"

namespace qutilites
{

QVBoxLayout* emptyVWidget(QWidget* parent,int margin)
{
    QWidget* widget = new QWidget(parent);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(margin, margin, margin, margin);

    return layout;
}

QHBoxLayout* emptyHWidget(QWidget* parent, int margin)
{
    QWidget* widget = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(margin, margin, margin, margin);

    return layout;
}

} // qutilites
