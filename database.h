#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>

class DataBase
{
public:
    enum EScoreFlags
    {
        eNone = 0,
        eIncoming = 0x01, // Доходы - зарплаты, планируемые доходы и т.д.
        eSpending = 0x02, // Расходы
        eActive = 0x04, // Активы - наличные, карты и пр.
        ePassive = 0x08, // Пассивы - кредиты, долги и пр.
    };

    struct SScore
    {
        SScore() : id(-1), parent_id(-1), flags(eNone) {}
        QString name;
        qint64 id;
        qint64 parent_id;
        quint64 flags;
        std::list<SScore> child;
        QDate added;
        QDate removed;
    };

    struct STransaction
    {
        STransaction() : score_id(-1), id(-1) {}
        QString desc;
        qint64 score_id;
        qint64 id;
        QString balance;
        QDate date;
    };

    DataBase();

    SScore scores();
    bool   setScore(const SScore& score, bool replace = false);
    bool   setScore(const QString& name, qint64 parent_id = -1, quint64 flags = eNone, const std::list<SScore>& childs = std::list<SScore>(), bool replace = false);
    bool   renameScore(const QString& old_name, const QString& new_name);
    bool   removeScore(qint64 id);

    std::list<STransaction> transacts(int year = -1, int month = -1, qint64 score_id = -1);
    bool setTransaction(const STransaction& tr, bool replace = false);
    bool setTransaction(const QDate& date, qint64 score_id, QString balance = QString(), QString desc = QString(), bool replace = false);
    bool removeTransaction(qint64 id);
    bool removeTransaction(const QDate& date, qint64 score_id);
    bool moveTransaction(const QDate& old_date, qint64 old_score_id, const QDate& new_date, qint64 new_score_id);

    static DataBase& inst()
    {
        static DataBase d;
        return d;
    }

private:
    bool         _recRemoveScore(qint64 id);
    void         _init();
    void         _createTables(const QString& connection);
    void         _clearMemoryCache();
    QStringList _dbDump();
    void        _memory_cache();
};

#endif // DATABASE_H
