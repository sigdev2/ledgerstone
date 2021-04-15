
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTreeView>
#include <QTableView>

#include "QutePtr.h"
#include "defaulttheme.h"

class ScoresModel;
class TransactionsModel;
class QSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

public slots:
    void updateScoreActions();
    void updateTransactionActions();

private slots:
    void addScore();
    void removeScore();
    void changeYear(int val);
    void scoresChanged(const QModelIndex& begin, const QModelIndex& end);
    void modelsErrors(const QString& title, const QString& text);
    void changeDescription();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);

private:
    void _updateMonthView();
    void _updateMonthViewHeaders();

    DefaultTheme m_defaultTheme;

    qt_ptr<QSpinBox>   m_years;

    qt_ptr<ScoresModel> m_scoresmodel;
    qt_ptr<QTreeView> m_scoresview;

    qt_ptr<TransactionsModel> m_monthmodel;
    qt_ptr<QTableView> m_monthview;
};

#endif // MAINWINDOW_H
