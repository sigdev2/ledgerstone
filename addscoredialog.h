#ifndef ADDSCOREDIALOG_H
#define ADDSCOREDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>

#include "QutePtr.h"

class AddScoreDialog : public QDialog
{
    Q_OBJECT
public:
    AddScoreDialog(QWidget* parent = NULL);
    ~AddScoreDialog() {};

    QString name() const { return m_name->text(); };
    quint64 flags() const;

private:
    qt_ptr<QLineEdit> m_name;
    qt_ptr<QRadioButton> m_none;
    qt_ptr<QRadioButton> m_active;
    qt_ptr<QRadioButton> m_passive;
    qt_ptr<QRadioButton> m_income;
    qt_ptr<QRadioButton> m_cost;
};

#endif // ADDSCOREDIALOG_H
