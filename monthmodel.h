#ifndef MONTHMODEL_H
#define MONTHMODEL_H

#include <list>
#include <map>

#include <QObject>
#include <QCalendar>

#include <unordered_map>

#include "database.h"
#include "monthheadermodel.h"
#include "calculateengine.h"
#include "theme.h"

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1,T2>& p) const
    {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2;
    }
};

class MonthModel : public QObject
{
    Q_OBJECT

public:
    enum ECellType
    {
        eResult = 0x10,
        eColResult = 0x20,
        eRowResult = 0x40,
        eSplitRow = 0x80,
        eHeader = 0x100,
        eCell = 0x200,
        eEmpty = 0x400
    };

    struct SItem
    {
        SItem() : editable(false),
            span(1),
            scoreType(DataBase::eNone),
            cellType(eEmpty),
            removed(false) {}

        QString expression;
        QString result;
        QString description;
        QString comment;
        bool    editable;
        int     span;
        quint64 scoreType;
        quint64 cellType;
        bool    removed;
    };

    MonthModel(MonthHeaderModel* header, int n, QObject* parent = NULL);
    MonthModel(const MonthModel& other);
    virtual ~MonthModel();

    void setHeader(MonthHeaderModel* header) { m_header = header; }

    QDate start() const { return m_start; }
    QDate end() const { return m_end; }

    Q_INVOKABLE int columnCount() const;
    Q_INVOKABLE int rowCount() const;

    ECellType dataType(int row, int col) const;
    const MonthModel::SItem& data(int row, int col) const;
    QDate date(int row) const;
    const MonthHeaderModel::SHeaderItem* headerItem(int row, int col) const;
    bool headerItemIsVisible(int row, int col) const;

    Q_INVOKABLE QString rowSumm(int row, quint64 scoresTypes = DataBase::eSpending) const;
    Q_INVOKABLE QString columnSumm(int col) const;
    Q_INVOKABLE QString summ(quint64 scoresTypes = DataBase::eSpending) const;
    Q_INVOKABLE QString summScoreChilds(int row, int col) const;
    Q_INVOKABLE QString columnSummAll(int col) const;

    static inline quint64 сumulativeScoreTypes() { return (DataBase::eActive | DataBase::ePassive); }
    static inline quint64 signedScoreTypes() { return (DataBase::eActive | DataBase::eIncoming); }
    static inline quint64 allScoreTypes() { return (DataBase::eActive | DataBase::ePassive | DataBase::eIncoming | DataBase::eSpending); }

    void clearCache() { m_cache.clear(); }

    int number() const { return m_end.month(); }

    bool setData(int row, int col, const QString& balance);
    bool setDescription(int row, int col, const QString& desc);

private:
    void _dataBuild();

    MonthHeaderModel* m_header;
    QDate m_start;
    QDate m_end;

    SItem m_nullItem;

    std::map<QDate, std::map<qint64, DataBase::STransaction> > m_data;
    mutable std::unordered_map<std::pair<int, int>, SItem, pair_hash> m_cache;

    CalculateEngine m_engine;
    mutable bool m_needRemoveFromEngine;

    QCalendar m_calendar;
};

#endif // MONTHMODEL_H
