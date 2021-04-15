#include "monthmodel.h"

#include <iterator>

MonthModel::MonthModel(MonthHeaderModel* header, int n, QObject* parent)
    : QObject(parent), m_header(header), m_needRemoveFromEngine(true)
{
    const int year = m_header->year();
    const QString strYear = QStringLiteral("%1").arg(year, 4, 10, QLatin1Char('0'));
    const QString strMonth = QStringLiteral("%1").arg(n, 2, 10, QLatin1Char('0'));
    const QString strLastDay = QStringLiteral("%1").arg(m_calendar.daysInMonth(n, year), 2, 10, QLatin1Char('0'));
    m_start = QDate::fromString(strYear + strMonth + "01", "yyyyMMdd");
    m_end = QDate::fromString(strYear + strMonth + strLastDay, "yyyyMMdd");

    if (m_header != NULL)
    {
        _dataBuild();
        m_engine.setMonthModel(year, n, this);
    }
}

MonthModel::MonthModel(const MonthModel& other)
   : QObject(other.parent()), m_header(other.m_header),
     m_start(other.m_start), m_end(other.m_end),
     m_data(other.m_data)
{
    other.m_needRemoveFromEngine = false;
    m_engine.setMonthModel(m_header->year(), m_start.month(), this);
}

MonthModel::~MonthModel()
{
    if (m_needRemoveFromEngine)
        m_engine.removeModel(m_header->year(), m_start.month());
}

int MonthModel::columnCount() const
{
    if (m_header == NULL)
        return 0;
    return m_header->columnCount() + 1 /* results */;
}

int MonthModel::rowCount() const
{
    if (m_header == NULL)
        return 0;
    return m_end.daysInMonth() + m_header->rowCount() * 2 + 1 /*splitrow*/;
}

MonthModel::ECellType MonthModel::dataType(int row, int col) const
{
    if (m_header == NULL)
        return eEmpty;

    const int last_col = columnCount() - 1;
    const int last_row = rowCount() - 1;

    const int header_size = m_header->rowCount();

    if (row < header_size)
    {
        // headers
        if (col == last_col)
            return eEmpty;
        return eHeader;
    }

    if (row >= m_end.daysInMonth() + header_size)
    {
        // results
        if (row == last_row)
            return eSplitRow;
        if (col == last_col)
            return eResult;
        return eColResult;
    }

    if (col == last_col)
        return eRowResult;
    return eCell;
}

const MonthModel::SItem& MonthModel::data(int row, int col) const
{
    const std::pair<int, int> point(row, col);
    std::unordered_map<std::pair<int, int>, SItem>::iterator cacheIt = m_cache.find(point);
    if (cacheIt != m_cache.end())
        return cacheIt->second;

    if (m_header == NULL)
        return m_nullItem;

    SItem item = m_nullItem;

    ECellType type = dataType(row, col);
    item.cellType = type;
    switch(type)
    {
        case eResult:
        {
            const int header_size = m_header->rowCount();
            if (row == m_end.daysInMonth() + header_size * 2 - 1)
                item.result = summ();
            item.scoreType = DataBase::eSpending;
            break;
        }
        case eColResult:
        {
            //const int header_size = m_header->rowCount();
            const MonthHeaderModel::SHeaderItem* header = headerItem(row, col);

            if (header != NULL)
            {
                //const int hedaerRow = row - m_end.daysInMonth() - header_size;

                const bool isCumulative = ((header->flags & сumulativeScoreTypes()) != 0);
                item.span = header->span;
                item.scoreType = header->flags;
                //if ((header->lvl == hedaerRow && header->hasChilds) || hedaerRow == header_size - 1)
                {
                    if (header->hasChilds)
                    {
                        item.result = summScoreChilds(row, col);
                    }
                    else
                    {
                        if (isCumulative)
                            item.result = columnSummAll(col);
                        else
                            item.result = columnSumm(col);
                    }
                }
                item.comment = header->name;

                item.removed = (header->removed.isValid() && header->removed <= m_end);
            }

            break;
        }
        case eRowResult:
        {
            item.result = rowSumm(row);
            item.scoreType = DataBase::eSpending;
            break;
        }
        case eSplitRow:
        {
            break;
        }
        case eHeader:
        {
            //const int header_size = m_header->rowCount();
            const MonthHeaderModel::SHeaderItem* header = headerItem(row, col);
            if (header != NULL)
            {
                item.span = header->span;
                //if (header->hasChilds || header->lvl == header_size - 1 || row == header_size - 1)
                    item.result = header->name;
            }
            break;
        }
        case eCell:
        {
            const int header_size = m_header->rowCount();
            const MonthHeaderModel::SHeaderItem* header = headerItem(row, col);

            if (header != NULL)
            {
                const QDate day = m_start.addDays(row - header_size);

                DataBase::STransaction t = m_data.at(day).at(header->id);
                item.expression = t.balance;
                item.result = m_engine.calculate(item.expression);

                item.description = t.desc;
                item.comment = t.desc;
                item.editable = true;
                item.scoreType = header->flags;

                item.removed = (header->removed.isValid() && header->removed <= day);
            }
            break;
        }
        case eEmpty:
        default:
        {
            break;
        }
    }

    m_cache.insert_or_assign(point, item);
    return m_cache[point];
}

QDate MonthModel::date(int row) const
{
    if (m_header == NULL)
        return QDate();

    if (dataType(row, 0) != eCell)
        return QDate();

    const int header_size = m_header->rowCount();
    return m_start.addDays(row - header_size);
}

const MonthHeaderModel::SHeaderItem* MonthModel::headerItem(int row, int col) const
{
    if (m_header == NULL)
        return NULL;

    const int header_width = m_header->columnCount();
    if (col < 0 || col > header_width)
        return NULL;
    const int header_col = col;

    const int rows = rowCount();

    const int header_height = m_header->rowCount();
    const int second_header_pos = rows - header_height - 1 /* sep row */;
    int header_row = row;

    if (row >= 0 && row < rows - 1 /* sep row */)
    {
        if (row >= header_height && row < second_header_pos)
        {
            header_row = header_height - 1;
        }
        else if (row >= second_header_pos && row < rows - 1 /* sep row */)
        {
            header_row = row - second_header_pos;
        }
    }
    else
    {
        header_row = header_height - 1;
    }

    return &(m_header->item(header_row, header_col));
}

bool MonthModel::headerItemIsVisible(int row, int col) const
{
    if (m_header == NULL)
        return false;

    const int header_width = m_header->columnCount();
    if (col < 0 || col > header_width)
        return false;
    const int header_col = col;

    const int rows = rowCount();
    const int header_height = m_header->rowCount();
    const int second_header_pos = rows - header_height - 1 /* sep row */;
    int header_row = row;

    if (row >= 0 && row < rows - 1 /* sep row */)
    {
        if (row >= header_height && row < second_header_pos)
        {
            return false;
        }
        else if (row >= second_header_pos && row < rows - 1 /* sep row */)
        {
            header_row = row - second_header_pos;
        }
    }
    else
    {
        return false;
    }

    const MonthHeaderModel::SHeaderItem& hedaerItem = m_header->item(header_row, header_col);
    if (!hedaerItem.hasChilds && hedaerItem.lvl != header_height - 1 && header_row != header_height - 1)
        return false;

    return true;
}

Q_INVOKABLE QString MonthModel::rowSumm(int row, quint64 scoresTypes) const
{
    if (m_header == NULL)
        return QString();

    const int cols = columnCount();
    const int last_col_size = cols - 1 /* results */;
    QString summ = QString::fromLatin1("0");
    bool hasData = false;
    for (int i = 0; i < last_col_size; ++i)
    {
        const MonthModel::SItem& item = data(row, i);
        if (!item.result.isEmpty())
        {
            if ((scoresTypes == DataBase::eNone && item.scoreType == DataBase::eNone) ||
                (item.scoreType & scoresTypes) != 0)
            {
                hasData = true;
                summ += QString::fromLatin1(" + (") + item.result + QString::fromLatin1(")");
            }
        }
    }

    if (!hasData)
        return QString();

    return m_engine.calculate(summ);
}

Q_INVOKABLE QString MonthModel::columnSumm(int col) const
{
    if (m_header == NULL)
        return QString();

    const int rows = rowCount();
    const int header_size = m_header->rowCount();
    const int last_row_size = rows - header_size - 1 /*splitrow*/;
    QString summ = QString::fromLatin1("0");
    bool hasData = false;
    for (int i = header_size /* header */; i < last_row_size; ++i)
    {
        const MonthModel::SItem& item = data(i, col);
        if (!item.result.isEmpty())
        {
            hasData = true;
            summ += QString::fromLatin1(" + (") + item.result + QString::fromLatin1(")");
        }
    }

    if (!hasData)
        return summ;

    return m_engine.calculate(summ);
}

QString MonthModel::summ(quint64 scoresTypes) const
{
    if (m_header == NULL)
        return QString();

    const int rows = rowCount();
    const int header_size = m_header->rowCount();
    const int last_row_size = rows - header_size - 1 /*splitrow*/;
    QString summ = QString::fromLatin1("0");
    bool hasData = false;
    for (int i = header_size /* header */; i < last_row_size; ++i)
    {
        const QString strRow = rowSumm(i, scoresTypes);
        if (!strRow.isEmpty())
        {
            hasData = true;
            summ += QString::fromLatin1(" + (") + strRow + QString::fromLatin1(")");
        }
    }

    if (!hasData)
        return summ;

    return m_engine.calculate(summ);
}

Q_INVOKABLE QString MonthModel::summScoreChilds(int row, int col) const
{
    if (m_header == NULL)
        return QString();

    if (ECellType::eColResult != dataType(row, col))
        return QString();

    const MonthHeaderModel::SHeaderItem* header = headerItem(row, col);

    if (header == NULL)
        return QString();

    if (!header->hasChilds || header->span == 1)
        return columnSumm(col);

    QString summ = QString::fromLatin1("0");
    bool hasData = false;
    const int max_col = col + header->span;

    for (int i = col; i < max_col;)
    {
        const MonthModel::SItem& item = data(row + 1, i);
        if (!item.result.isEmpty())
        {
            if (header->flags == DataBase::eNone ||
                (item.scoreType & header->flags) != 0)
            {
                hasData = true;
                QString sign = QString::fromLatin1("+");
                if (header->flags == DataBase::eNone &&
                    (item.scoreType & (DataBase::eSpending | DataBase::ePassive)) != 0)
                    sign = QString::fromLatin1("-");

                summ += QString::fromLatin1(" ") + sign + QString::fromLatin1(" (") + item.result + QString::fromLatin1(")");
            }
        }
        i += item.span;
    }

    if (!hasData)
        return summ;

    return m_engine.calculate(summ);
}

QString MonthModel::columnSummAll(int col) const
{
    if (m_header == NULL)
        return QString();

    const MonthHeaderModel::SHeaderItem* header = headerItem(-1, col);
    if (header == NULL)
        return QString();

    QString summ = QString::fromLatin1("0");
    bool hasData = false;
    std::list<DataBase::STransaction> trs = DataBase::inst().transacts(-1, -1, header->id);
    for(std::list<DataBase::STransaction>::iterator it = trs.begin(); it != trs.end(); ++it)
    {
        if (it->date > m_end)
            continue;

        if (!it->balance.isEmpty())
        {
            hasData = true;
            summ += QString::fromLatin1(" + (") + it->balance + QString::fromLatin1(")");
        }
    }

    if (!hasData)
        return QString();

    return m_engine.calculate(summ);
}

bool MonthModel::setData(int row, int col, const QString& balance)
{
    if (m_header == NULL)
        return false;

    m_cache.clear();

    const int header_size = m_header->rowCount();
    DataBase::STransaction& t = m_data.at(m_start.addDays(row - header_size)).at(m_header->scoreId(header_size - 1, col));
    t.balance = balance;
    return DataBase::inst().setTransaction(t, true);
}

bool MonthModel::setDescription(int row, int col, const QString& desc)
{
    if (m_header == NULL)
        return false;

    m_cache.clear();

    const int header_size = m_header->rowCount();
    DataBase::STransaction& t = m_data.at(m_start.addDays(row - header_size)).at(m_header->scoreId(header_size - 1, col));
    t.desc = desc;
    return DataBase::inst().setTransaction(t, true);
}

void MonthModel::_dataBuild()
{
    QDate iterateDate = m_start;
    const int header_size = m_header->rowCount();
    const int columns = m_header->columnCount();
    while(iterateDate <= m_end )
    {
        std::map<qint64, DataBase::STransaction> cells;
        for (int col = 0; col < columns; ++col)
        {
            const qint64 scoreId = m_header->scoreId(header_size - 1, col);
            DataBase::STransaction emptyItem;
            emptyItem.date = iterateDate;
            emptyItem.score_id = scoreId;
            cells[scoreId] = emptyItem;
        }
        m_data[iterateDate] = cells;
        iterateDate = iterateDate.addDays(1);
    }

    std::list<DataBase::STransaction> trs = DataBase::inst().transacts(m_header->year(), m_start.month());
    for(std::list<DataBase::STransaction>::iterator it = trs.begin(); it != trs.end(); ++it)
        m_data[it->date][it->score_id] = *it;
}
