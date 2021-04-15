
#include "scoresmodel.h"

ScoresModel::ScoresModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    m_scores = DataBase::inst().scores();
}

QVariant ScoresModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DataBase::SScore* score = reinterpret_cast<DataBase::SScore*>(index.internalPointer());
    if (score->removed.isValid() && score->removed < QDate::currentDate())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    QString type;
    switch(score->flags)
    {
        default:
        case DataBase::eNone: type = QString::fromLatin1("n"); break;
        case DataBase::eIncoming: type = QString::fromLatin1("i"); break;
        case DataBase::eSpending: type = QString::fromLatin1("s"); break;
        case DataBase::eActive: type = QString::fromLatin1("a"); break;
        case DataBase::ePassive: type = QString::fromLatin1("p"); break;
    }

    return QString::fromLatin1("[") + type + QString::fromLatin1("] ") + score->name;
}

QVariant ScoresModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
        return tr("Scores");

    return QVariant();
}

QModelIndex ScoresModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    if (row < 0 || column < 0)
        return QModelIndex();

    if (parent.isValid())
    {
        DataBase::SScore* root = reinterpret_cast<DataBase::SScore*>(parent.internalPointer());
        if (root->removed.isValid() && root->removed < QDate::currentDate())
            return QModelIndex();

        if (root->child.empty())
            return QModelIndex();

        std::list<DataBase::SScore>::iterator it = root->child.begin();
        std::advance(it, row);

        if (it->removed.isValid() && it->removed < QDate::currentDate())
            return QModelIndex();

        return createIndex(row, column, &(*it));
    }

    if (m_scores.child.empty())
        return QModelIndex();

    std::list<DataBase::SScore>::const_iterator it = m_scores.child.begin();
    std::advance(it, row);

    if (it->removed.isValid() && it->removed < QDate::currentDate())
        return QModelIndex();

    return createIndex(row, column, const_cast<DataBase::SScore*>(&(*it)));
}

QModelIndex ScoresModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    DataBase::SScore* s = reinterpret_cast<DataBase::SScore*>(index.internalPointer());
    if (s->parent_id == -1)
        return QModelIndex();

    if (s->removed.isValid() && s->removed < QDate::currentDate())
        return QModelIndex();

    DataBase::SScore* ret = NULL;
    int row = 0;
    if (!_find(s->parent_id, m_scores, 0, &ret, row))
        return QModelIndex();

    if (ret == NULL)
        return QModelIndex();

    if (ret->removed.isValid() && ret->removed < QDate::currentDate())
        return QModelIndex();

    return createIndex(row, 0, ret);
}

int ScoresModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

int ScoresModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return static_cast<int>(m_scores.child.size());

    const DataBase::SScore* s = reinterpret_cast<DataBase::SScore*>(parent.internalPointer());

    if (s->removed.isValid() && s->removed < QDate::currentDate())
        return 0;

    return static_cast<int>(s->child.size());
}

Qt::ItemFlags ScoresModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    DataBase::SScore* score = reinterpret_cast<DataBase::SScore*>(index.internalPointer());
    if (score->removed.isValid() && score->removed < QDate::currentDate())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

bool ScoresModel::addScore(const QString& name, quint64 flags, const QModelIndex& parent)
{
    qint64 parent_id = -1;
    qint64 size = m_scores.child.size();
    if (parent.isValid())
    {
        DataBase::SScore* score = reinterpret_cast<DataBase::SScore*>(parent.internalPointer());
        if (score->removed.isValid() && score->removed < QDate::currentDate())
            return false;
        parent_id = score->id;
        size = score->child.size();
    }

    if (!DataBase::inst().setScore(name, parent_id, flags))
    {
        emit error(tr("Add score"),
                   tr("An error has occurred while adding the score. The database may be corrupted."));
        return false;
    }

    beginResetModel();
        m_scores = DataBase::inst().scores();
    endResetModel();

    return true;
}

bool ScoresModel::removeScore(const QModelIndex& index)
{
    if (!index.isValid())
        return false;

    DataBase::SScore* score = reinterpret_cast<DataBase::SScore*>(index.internalPointer());
    if (score->removed.isValid() && score->removed < QDate::currentDate())
        return false;

    if (!DataBase::inst().removeScore(score->id))
    {
        emit error(tr("Remove score"),
                   tr("An error has occurred while deleting the score. The database may be corrupted."));
        return false;
    }

    beginResetModel();
        m_scores = DataBase::inst().scores();
    endResetModel();

    return true;
}

bool ScoresModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (!index.isValid())
        return false;

    DataBase::SScore* score = reinterpret_cast<DataBase::SScore*>(index.internalPointer());
    if (score->removed.isValid() && score->removed < QDate::currentDate())
        return false;

    if (!DataBase::inst().renameScore(score->name, value.toString()))
    {
        emit error(tr("Change score name"),
                   tr("An error has occurred while changing the score name. The database may be corrupted."));
        return false;
    }

    score->name = value.toString();

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return true;
}

bool ScoresModel::_find(qint64 id, const DataBase::SScore& root, int pos, DataBase::SScore** retScore, int& retRow) const
{
    if (root.id == id)
    {
        *retScore = const_cast<DataBase::SScore*>(&root);
        retRow = pos;
        return true;
    }

    int r = 0;
    for (std::list<DataBase::SScore>::const_iterator it = root.child.begin(); it != root.child.end(); ++it)
    {
        if (_find(id, *it, r, retScore, retRow))
            return true;
        ++r;
    }

    return false;
}
