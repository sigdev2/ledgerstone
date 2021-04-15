#ifndef Q_UTILITES_H
#define Q_UTILITES_H

class QVBoxLayout;
class QHBoxLayout;
class QWidget;

namespace qutilites
{
    QVBoxLayout* emptyVWidget(QWidget* parent, int margin = 0);
    QHBoxLayout* emptyHWidget(QWidget* parent, int margin = 0);

} // qutilites

#endif // Q_UTILITES_H
