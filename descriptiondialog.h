#ifndef DESCRIPTIONDIALOG_H
#define DESCRIPTIONDIALOG_H

#include <QDialog>
#include <QTextEdit>

#include "QutePtr.h"

class DescriptionDialog : public QDialog
{
    Q_OBJECT
public:
    DescriptionDialog(const QString& desc, QWidget* parent = NULL);
    ~DescriptionDialog() {};

    QString description() const { return m_desc->toPlainText(); };

private:
    qt_ptr<QTextEdit> m_desc;
};

#endif // DESCRIPTIONDIALOG_H
