
#include <QVBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QCoreApplication>
#include <QSpinBox>
#include <QSplitter>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QMouseEvent>

#include "addscoredialog.h"

#include "qutilites.h"
#include "actions.h"

#include "mainwindow.h"
#include "scoresmodel.h"
#include "transactionsmodel.h"
#include "descriptiondialog.h"

#include "QutePtr.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // menu bar

    QMenuBar* menu = new QMenuBar(this);
    QActionMenuBar mainMenuActions(menu);
    mainMenuActions.addActions("File: Close < Scores: AddScore RemoveScore < Transaction: Description");
    setMenuBar(menu);

    Actions* actions = Actions::instance();

    // main layout

    QSplitter* split = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(split);

    // Scores widget

    m_scoresview = new QTreeView(this);
    m_scoresmodel = new ScoresModel(m_scoresview);
    m_scoresview->setModel(m_scoresmodel);
    m_scoresview->setStyleSheet("font-size: 12pt;");

    connect(m_scoresmodel, &ScoresModel::dataChanged, this, &MainWindow::scoresChanged);
    connect(m_scoresmodel, &ScoresModel::error, this, &MainWindow::modelsErrors);

    for (int column = 0; column < m_scoresmodel->columnCount(); ++column)
        m_scoresview->resizeColumnToContents(column);

    connect(actions->get("Close"), &QAction::triggered, qApp, &QCoreApplication::quit);

    connect(m_scoresview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateScoreActions);

    connect(mainMenuActions.getMenu("Scores"), &QMenu::aboutToShow, this, &MainWindow::updateScoreActions);
    connect(mainMenuActions.getMenu("Transaction"), &QMenu::aboutToShow, this, &MainWindow::updateTransactionActions);
    connect(actions->get("AddScore"), &QAction::triggered, this, &MainWindow::addScore);
    connect(actions->get("RemoveScore"), &QAction::triggered, this, &MainWindow::removeScore);
    connect(actions->get("Description"), &QAction::triggered, this, &MainWindow::changeDescription);

    split->addWidget(m_scoresview);
    m_scoresview->expandAll();
    m_scoresview->viewport()->installEventFilter(this);

    // Month widget

    QVBoxLayout* dates = qutilites::emptyVWidget(this);

    QHBoxLayout* spinBoxLayout = new QHBoxLayout();
    m_years = new QSpinBox(this);
    m_years->setStyleSheet("font-size: 14pt;");
    m_years->setRange(1, 9999);
    m_years->setValue(QDate::currentDate().year());
    QLabel* yearSpinLabel = new QLabel(tr("Year: "), this);
    yearSpinLabel->setStyleSheet("font-size: 14pt;");
    spinBoxLayout->addWidget(yearSpinLabel);
    spinBoxLayout->addWidget(m_years);
    spinBoxLayout->addStretch(1);

    dates->addLayout(spinBoxLayout);

    m_monthview = new QTableView(this);
    m_monthview->setSelectionBehavior(QTableView::SelectRows);
    m_monthmodel = new TransactionsModel(&m_defaultTheme, m_monthview);
    m_monthview->setModel(m_monthmodel);
    m_monthview->setStyleSheet("font-size: 12px;");
    m_monthview->horizontalHeader()->setMinimumSectionSize(20);
    m_monthview->horizontalHeader()->setMaximumSectionSize(85);
    m_monthview->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_monthview->setTextElideMode(Qt::ElideNone);

    connect(m_monthmodel, &TransactionsModel::error, this, &MainWindow::modelsErrors);
    connect(m_monthview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateTransactionActions);

    dates->addWidget(m_monthview);

    split->addWidget(dates->parentWidget());

    connect(m_years, SIGNAL(valueChanged(int)), this, SLOT(changeYear(int)));

    // sizes

    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 5);

    // update

    updateTransactionActions();
    updateScoreActions();
    _updateMonthView();

    showMaximized();
}

void MainWindow::addScore()
{
    QModelIndex index = m_scoresview->selectionModel()->currentIndex();
    if (m_scoresview->selectionModel()->selection().isEmpty())
        index = QModelIndex();

    AddScoreDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        const QString name = dlg.name();
        const quint64 f = dlg.flags();
        if (!name.isEmpty())
        {
            m_scoresmodel->addScore(name, f, index);
            m_scoresview->expandAll();
            _updateMonthView();
        }
    }
}

void MainWindow::removeScore()
{
    QModelIndex index = m_scoresview->selectionModel()->currentIndex();
    if (m_scoresview->selectionModel()->selection().isEmpty())
        index = QModelIndex();

    if (!index.isValid())
        return;

    if (QMessageBox::Yes == QMessageBox::question(this, tr("Remove score"),
                                                  tr("Deleting an score will also delete all child scores. Are you sure you want to do this?")))
    {
        m_scoresmodel->removeScore(index);
        m_scoresview->expandAll();
        _updateMonthView();
    }
}

void MainWindow::changeYear(int val)
{
    m_monthmodel->setYear(val);
    _updateMonthViewHeaders();
}

void MainWindow::scoresChanged(const QModelIndex& begin, const QModelIndex& end)
{
    Q_UNUSED(begin);
    Q_UNUSED(end);
    _updateMonthView();
}

void MainWindow::modelsErrors(const QString& title, const QString& text)
{
    QMessageBox::critical(this, title, text);
}

void MainWindow::changeDescription()
{
    if (m_monthview->selectionModel()->selection().isEmpty())
        return;
    QModelIndex index = m_monthview->selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    QString oldDesc = m_monthmodel->description(index);
    DescriptionDialog dlg(oldDesc, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        const QString desc = dlg.description();
        if (!desc.isEmpty() && oldDesc != desc)
            m_monthmodel->setDescription(index, desc);
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_scoresview->viewport() && event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(event);
        QModelIndex index = m_scoresview->indexAt(e->pos());
        if (!index.isValid())
        {
            m_scoresview->clearSelection();
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::updateScoreActions()
{
    Actions* actions = Actions::instance();
    const bool hasSelection = !m_scoresview->selectionModel()->selection().isEmpty();
    actions->get("RemoveScore")->setEnabled(hasSelection);

    const QModelIndex current = m_scoresview->selectionModel()->currentIndex();
    if (current.isValid())
        m_scoresview->closePersistentEditor(current);
}

void MainWindow::updateTransactionActions()
{
    Actions* actions = Actions::instance();
    bool validIndex = false;
    if (!m_monthview->selectionModel()->selection().isEmpty())
    {
        const QModelIndex current = m_monthview->selectionModel()->currentIndex();
        if (current.isValid())
        {
            validIndex = (m_monthmodel->flags(current) & Qt::ItemIsEditable) != 0;
            m_monthview->closePersistentEditor(current);
        }
    }

    actions->get("Description")->setEnabled(validIndex);
}

void MainWindow::_updateMonthView()
{
    m_monthmodel->setScores(m_scoresmodel->scores());
    _updateMonthViewHeaders();
}

void MainWindow::_updateMonthViewHeaders()
{
    QHeaderView* header = m_monthview->horizontalHeader();
    if (header != NULL)
    {
        const int size = header->count();
        header->resizeSections(QHeaderView::ResizeToContents);
        for (int c = 0; c < size - 1; ++c)
            header->setSectionResizeMode(c, QHeaderView::Interactive);
        header->setSectionResizeMode(size - 1, QHeaderView::Stretch);
    }

    // set col's span
    m_monthview->clearSpans();
    const int rows = m_monthmodel->rowCount();
    const int cols = m_monthmodel->columnCount();
    for (int row = 0; row < rows; ++row)
        for (int col = 0; col < cols;)
        {
            const QModelIndex idx = m_monthmodel->index(row, col);
            const QSize span = m_monthmodel->span(idx);
            if (span.width() != 1)
                m_monthview->setSpan(row, col, span.height(), span.width());
            col += span.width();
        }
}
