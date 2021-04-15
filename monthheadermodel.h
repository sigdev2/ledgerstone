#ifndef MONTHHEADERMODEL_H
#define MONTHHEADERMODEL_H

#include <QDate>

#include "database.h"

class MonthHeaderModel
{
public:
    struct SHeaderItem
    {
        SHeaderItem() : hasChilds(false), hasParent(false), lvl(0), span(1), id(-1), flags(DataBase::eNone) {}
        QString name;
        bool hasChilds;
        bool hasParent;
        QDate removed;
        int lvl;
        int span;
        int id;
        quint64 flags;
    };

    MonthHeaderModel(int year = QDate::currentDate().year(), DataBase::SScore* scores = NULL);
    virtual ~MonthHeaderModel() {};

    int scoreId(int row, int col) const;
    int span(int row, int col) const;
    QString data(int row, int col) const;
    const SHeaderItem& item(int row, int col) const;

    int columnCount() const;
    int rowCount() const;

    QStringList headers() const { return m_headers; };

    int year() const { return m_year; }

private:
    int _build(const DataBase::SScore& root, int lvl = 0);

    DataBase::SScore* m_scores;
    int m_year;

    QDate m_start;
    QDate m_end;

    SHeaderItem m_nullItem;
    std::vector<std::vector<SHeaderItem> > m_table;

    QStringList m_headers;
    int m_height;
    int m_width;
};

#endif // MONTHHEADERMODEL_H
