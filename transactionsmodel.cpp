
#include "transactionsmodel.h"

#include "defaulttheme.h"

TransactionsModel::TransactionsModel(Theme* theme, QObject* parent)
    : QAbstractItemModel(parent), m_scores(NULL), m_theme(theme), m_rows(0)
{
    m_month.reserve(12);
    m_rowsInMonth.reserve(12);
}

TransactionsModel::~TransactionsModel()
{
    m_month.clear();
}

QVariant TransactionsModel::data(const QModelIndex& index, int role) const
{
    const int month = monthForIdx(index);
    if (month < 0)
        return QVariant();

    const int col = index.column();
    const int row = index.row() - m_rowsInMonth[month].first;

    const MonthModel& model = m_month.at(month);
    const MonthModel::SItem& item = model.data(row, col);

    if (role == Qt::DisplayRole)
    {
        QString ret = item.result;
        if (!ret.isEmpty() &&
            (item.scoreType & model.signedScoreTypes()) != 0 &&
            ret[0].isDigit() &&
            ret != QString::fromLatin1("0.00") &&
            ret != QString::fromLatin1("0"))
            ret = QString::fromLatin1(" + ") + ret;
        if ((item.cellType & (MonthModel::eColResult | MonthModel::eHeader)) == 0)
            return ret;

        const MonthHeaderModel::SHeaderItem* hedaerItem = model.headerItem(row, col);
        if (hedaerItem == NULL)
            return ret;

        if ((item.cellType & MonthModel::eColResult) != 0)
        {
            if (!model.headerItemIsVisible(row, col))
                return QString();

            return hedaerItem->name + QString::fromLatin1(":\n") + ret;
        }
        else if ((item.cellType & MonthModel::eHeader) != 0)
        {
            if (!model.headerItemIsVisible(row, col))
            {
                if (row == 0)
                    return QString::fromLatin1("-");
                else
                    return QString::fromLatin1("|");
            }
        }

        return ret;
    }

    if (role == Qt::EditRole)
        return item.expression;

    if (role == Qt::ToolTipRole)
    {
        if (item.expression.isEmpty() && item.comment.isEmpty())
            return QVariant();

        QString toolTip = item.expression;
        if (item.expression.isEmpty())
            toolTip = item.result;
        else if (item.expression != QString::fromLatin1("0.00") &&
                 item.expression != QString::fromLatin1("0") &&
                 item.expression != item.result &&
                 item.expression != item.result + QString::fromLatin1(".00"))
                     toolTip += QString::fromLatin1(" = ") + item.result;

        return toolTip +
               (!toolTip.isEmpty() && !item.comment.isEmpty() ? QString::fromLatin1("\n--------\n") : QString()) +
               item.comment;
    }

    if ((role != Qt::BackgroundColorRole && role != Qt::TextColorRole) || m_theme == NULL)
        return QVariant();

    const Theme::ERole r = (role == Qt::BackgroundColorRole ? Theme::eBackground : Theme::eFont);
    quint64 colorId = item.cellType | item.scoreType;

    if (item.removed)
        colorId |= DefaultTheme::eRemoved;

    QColor color = m_theme->color(r, colorId, item.result);
    if (!color.isValid())
        return QVariant();
    return color;
}

QVariant TransactionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole || role == Qt::SizeHintRole) && !m_month.empty())
    {
        if (orientation == Qt::Horizontal)
        {
            if (role == Qt::SizeHintRole)
            {
                return QVariant(QSize(20, 22));
            }
            else
            {
                if (section < m_header.headers().size())
                    return m_header.headers().at(section);

                return QObject::tr("Results");
            }
        }
        else
        {
            if (role == Qt::SizeHintRole)
            {
                return QVariant(QSize(70, 22));
            }
            else
            {
                const int month = monthForIdx(section);
                if (month < 0)
                    return QVariant();
                const QDate date = m_month.at(month).date(section - m_rowsInMonth[month].first);
                if (!date.isValid())
                    return QVariant();
                return date.toString("yyyy.MM.dd");
            }
        }
    }

    return QVariant();
}

QModelIndex TransactionsModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TransactionsModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int TransactionsModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    if (m_month.empty())
        return 0;
    return m_month.begin()->columnCount();
}

QSize TransactionsModel::span(const QModelIndex& index) const
{
    const int month = monthForIdx(index);
    if (month < 0)
        return QSize(1, 1);

    const int col = index.column();
    const int row = index.row() - m_rowsInMonth[month].first;

    const MonthModel::SItem& item = m_month.at(month).data(row, col);
    return QSize(item.span, 1);
}

int TransactionsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    if (m_month.empty())
        return 0;
    return m_rows;
}

Qt::ItemFlags TransactionsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    const int month = monthForIdx(index);
    if (month < 0)
        return Qt::NoItemFlags;

    const int col = index.column();
    const int row = index.row() - m_rowsInMonth[month].first;

    const MonthModel::SItem& item = m_month.at(month).data(row, col);
    if (item.editable)
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);

    return QAbstractItemModel::flags(index);
}

QString TransactionsModel::description(const QModelIndex& index) const
{
    if (!index.isValid())
        return QString();
    const int month = monthForIdx(index);
    if (month < 0)
        return QString();

    const int col = index.column();
    const int row = index.row() - m_rowsInMonth[month].first;

    return m_month.at(month).data(row, col).description;
}

bool TransactionsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole)
        return false;

    const int month = monthForIdx(index);
    if (month < 0)
        return false;

    const int col = index.column();
    const int row = index.row() - m_rowsInMonth[month].first;

    MonthModel& month_model = m_month.at(month);
    const MonthModel::SItem& item = month_model.data(row, col);

    if (!item.editable)
        return false;

    if (item.scoreType & month_model.сumulativeScoreTypes())
        for (std::vector<MonthModel>::iterator mIt = m_month.begin(); mIt != m_month.end(); ++mIt)
            if (mIt->number() != month + 1)
                 mIt->clearCache();

    if (!month_model.setData(row, col, value.toString()))
    {
        emit error(tr("Change transaction"),
                   tr("An error has occurred while changing the transaction. The database may be corrupted."));
        return false;
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::ToolTipRole});

    return true;
}

bool TransactionsModel::setDescription(const QModelIndex& index, const QString& desc)
{
    const int month = monthForIdx(index);
    if (month < 0)
        return false;

    const int col = index.column();
    const int row = index.row() - m_rowsInMonth[month].first;

    MonthModel& month_model = m_month.at(month);
    const MonthModel::SItem& item = month_model.data(row, col);

    if (!item.editable)
        return false;

    if (!month_model.setDescription(row, col, desc))
    {
        emit error(tr("Change description"),
                   tr("An error has occurred while changing description of the transaction. The database may be corrupted."));
        return false;
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::ToolTipRole});

    return true;
}

void TransactionsModel::setYear(int year)
{
    if (m_scores != NULL)
    {
        beginResetModel();
             m_header = MonthHeaderModel(year, m_scores);
            _initMonth();
        endResetModel();
    }
}

void TransactionsModel::setScores(DataBase::SScore* scores)
{
    beginResetModel();
        m_scores = scores;
        m_header = MonthHeaderModel(m_header.year(), m_scores);
        _initMonth();
    endResetModel();
}

void TransactionsModel::setTheme(Theme* theme)
{
    beginResetModel();
    m_theme = theme;
    endResetModel();
}

void TransactionsModel::_initMonth()
{
    m_month.clear();
    m_rowsInMonth.clear();
    m_rows = 0;
    for (int i = 1; i <= 12; ++i)
    {
        MonthModel m(&m_header, i);
        const int rows = m.rowCount();
        m_rowsInMonth.push_back(std::pair<int, int>(m_rows, m_rows + rows - 1));
        m_rows += rows;
        m_month.push_back(m);
    }
}

int TransactionsModel::monthForIdx(const QModelIndex& index) const
{
    if (!index.isValid())
        return -1;

    return monthForIdx(index.row());
}

int TransactionsModel::monthForIdx(int idx_row) const
{
    int month = 0;
    for (std::vector<std::pair<int, int> >::const_iterator it = m_rowsInMonth.begin(); it != m_rowsInMonth.end(); ++it)
    {
        if (idx_row >= it->first && idx_row <= it->second)
            break;
        ++month;
    }

    if (month == static_cast<int>(m_rowsInMonth.size()))
        return -1;
    return month;
}
