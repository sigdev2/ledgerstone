
#ifndef SCORESMODEL_H
#define SCORESMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "database.h"

class ScoresModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ScoresModel(QObject* parent = NULL);
    ~ScoresModel() {}

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    DataBase::SScore* scores() { return &m_scores; }

    bool addScore(const QString& name, quint64 flags = DataBase::eNone, const QModelIndex& parent = QModelIndex());
    bool removeScore(const QModelIndex& index);
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

signals:
    void error(const QString& title, const QString& text);

private:
    bool _find(qint64 id, const DataBase::SScore& root, int pos, DataBase::SScore** retScore, int& retRow) const;

    DataBase::SScore m_scores;
};

#endif // SCORESMODEL_H
