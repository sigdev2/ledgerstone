#include "monthheadermodel.h"

MonthHeaderModel::MonthHeaderModel(int year, DataBase::SScore* scores)
    : m_scores(scores), m_year(year), m_height(0), m_width(0)
{
    const QString strYear = QStringLiteral("%1").arg(year, 4, 10, QLatin1Char('0'));
    m_start = QDate::fromString(strYear + "0101", "yyyyMMdd");
    m_end = QDate::fromString(strYear + "1231", "yyyyMMdd");

    if (m_scores != NULL)
    {
        m_table.reserve(4);
        m_width = _build(*m_scores);
        m_height = static_cast<int>(m_table.size());
    }
}

int MonthHeaderModel::scoreId(int row, int col) const
{
    return item(row, col).id;
}

int MonthHeaderModel::span(int row, int col) const
{
    return item(row, col).span;
}

QString MonthHeaderModel::data(int row, int col) const
{
    return item(row, col).name;
}

const MonthHeaderModel::SHeaderItem& MonthHeaderModel::item(int row, int col) const
{
    if (m_height == 0)
        return m_nullItem;
    if (row >= m_height)
        row = m_height - 1;

    if (m_width == 0)
        return m_nullItem;
    if (col >= m_width)
        col = m_width - 1;

    while(true)
    {
        const int s = static_cast<int>(m_table[row].size());
        if (row < 0 || (s != 0 && col < s))
            break;
        row -= 1;
    }

    if (row < 0)
        return m_nullItem;

    return m_table[row][col];
}

int MonthHeaderModel::columnCount() const
{
    return m_width;
}

int MonthHeaderModel::rowCount() const
{
    return m_height;
}

int MonthHeaderModel::_build(const DataBase::SScore& root, int lvl)
{
    if (static_cast<int>(m_table.size() - 1) < lvl)
    {
        std::vector<SHeaderItem> temp;
        temp.reserve(10);
        m_table.push_back(temp);
    }

    int span = 0;
    for (std::list<DataBase::SScore>::const_iterator it = root.child.begin(); it != root.child.end(); ++it)
    {
        const DataBase::SScore& score = *it;
        if (score.removed.isValid() && score.removed < m_start && score.removed > m_end)
            continue;

        if (lvl != 0)
        {
            while(true)
            {
                const size_t col = m_table[lvl].size();
                if (m_table[lvl - 1].size() > col)
                {
                    if (!m_table[lvl - 1][col].hasChilds)
                    {
                        m_table[lvl].push_back(m_table[lvl - 1][col]);
                        continue;
                    }
                }

                break;
            }
        }

        int local_span = 1;
        const bool hasChilds = score.child.size() > 0;
        if (hasChilds)
            local_span = _build(score, lvl + 1);
        else
            m_headers.push_back(score.name);

        SHeaderItem item;
        item.id = score.id;
        item.name = score.name;
        item.hasChilds = hasChilds;
        item.hasParent = (score.parent_id != -1);
        item.removed = score.removed;
        item.lvl = lvl;
        item.span = local_span;
        item.flags = score.flags;

        for (int i = 0; i < item.span; ++i)
            m_table[lvl].push_back(item);

        span += local_span;
    }

    return span;
}
