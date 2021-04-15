
#ifndef TRANSACTIONSMODEL_H
#define TRANSACTIONSMODEL_H

#include <vector>

#include <QAbstractItemModel>

#include "database.h"
#include "monthmodel.h"
#include "monthheadermodel.h"
#include "theme.h"

class TransactionsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TransactionsModel(Theme* theme, QObject* parent = NULL);
    virtual ~TransactionsModel();

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QSize span(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QString description(const QModelIndex& index) const;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    bool setDescription(const QModelIndex& index, const QString& desc);

public slots:
    void setYear(int year);
    void setScores(DataBase::SScore* scores);
    void setTheme(Theme* theme);

signals:
    void error(const QString& title, const QString& text);

private:
    void _initMonth();
    int  monthForIdx(const QModelIndex& index) const;
    int  monthForIdx(int idx_row) const;

    MonthHeaderModel        m_header;
    std::vector<MonthModel> m_month;
    DataBase::SScore*       m_scores;

    Theme* m_theme;

    std::vector<std::pair<int, int> > m_rowsInMonth;

    int m_rows;
};

#endif // TRANSACTIONSMODEL_H
