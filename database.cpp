#include "database.h"

#include <unordered_map>
#include <QtSql>

DataBase::DataBase()
{
    _init();
}

DataBase::SScore DataBase::scores()
{
    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
        return DataBase::SScore();

    QString q = QString::fromLatin1("SELECT * FROM SCORES");

    DataBase::SScore root;
    QSqlQuery query(q, memorydb);
    if ((query.exec() && query.isValid()) || query.lastError().text().isEmpty())
    {
        const int idIdx = query.record().indexOf("ID");
        const int parntIdIdx = query.record().indexOf("PARENT_ID");
        const int flagsIdx = query.record().indexOf("FLAGS");
        const int nameIdx = query.record().indexOf("NAME");
        const int addedIdx = query.record().indexOf("ADDED");
        const int removedIdx = query.record().indexOf("REMOVED");

        std::list<DataBase::SScore> toPush;
        std::unordered_map<quint64, DataBase::SScore*> scores;
        scores[-1] = &root;

        while (query.next())
        {
            bool success = false;
            DataBase::SScore s;
            s.id = query.value(idIdx).toInt(&success);
            if (success == false)
                continue;

            if ( query.value(parntIdIdx).isNull())
            {
                s.parent_id = -1;
            }
            else
            {
                success = false;
                s.parent_id = query.value(parntIdIdx).toInt(&success);
                if (success == false)
                    continue;
            }

            success = false;
            s.flags = query.value(flagsIdx).toUInt(&success);
            if (success == false)
                continue;

            s.name = query.value(nameIdx).toString();

            s.added = query.value(addedIdx).toDate();

            s.removed = query.value(removedIdx).toDate();

            std::unordered_map<quint64, DataBase::SScore*>::iterator it = scores.find(s.parent_id);
            if (it != scores.end())
            {
                it->second->child.push_back(s);
                scores[s.id] = &it->second->child.back();
            }
            else
            {
                toPush.push_back(s);
                scores[s.id] = &toPush.back();
            }
        }

        for (std::list<DataBase::SScore>::iterator sIt = toPush.begin(); sIt != toPush.end(); ++sIt)
        {
            std::unordered_map<quint64, DataBase::SScore*>::iterator it = scores.find(sIt->parent_id);
            if (it != scores.end())
                it->second->child.push_back(*sIt);
            else
                qDebug() << "Score structure is broken : " << sIt->name << " (" << sIt->id << " with parent " << sIt->parent_id << ")";
        }
    }
    else
    {
        qDebug() << "cache_connection : " << query.lastError().text() << query.lastQuery();
    }

    return root;
}

bool DataBase::setScore(const SScore& score, bool replace)
{
    return setScore(score.name, score.parent_id, score.flags, score.child, replace);
}

bool DataBase::setScore(const QString& name, qint64 parent_id, quint64 flags, const std::list<SScore>& childs, bool replace)
{
    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.isOpen())
        if (!db.open(QString(), QString()))
            return false;

    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
    {
        db.close();
        return false;
    }

    bool ret = false;

    QString q = QString::fromLatin1("SELECT * FROM SCORES WHERE NAME = ?");
    QSqlQuery query(q, memorydb);
    query.bindValue(0, name);
    if ((query.exec() && query.isValid()) || query.lastError().text().isEmpty())
    {
        const int parntIdIdx = query.record().indexOf("PARENT_ID");
        const int flagsIdx = query.record().indexOf("FLAGS");
        const int idIdx = query.record().indexOf("ID");
        if (query.next())
        {
            bool hasChanges = false;
            do
            {
                bool success = false;
                const qint64 ex_id = query.value(idIdx).toUInt(&success);
                if (success == false)
                    continue;

                if (!replace)
                {
                    success = false;
                    if (parent_id == -1)
                    {
                        qint64 ex_parent_id = -1;
                        if (!query.value(parntIdIdx).isNull())
                        {
                            success = false;
                            ex_parent_id = query.value(parntIdIdx).toInt(&success);
                            if (success == false)
                                continue;
                            if (ex_parent_id != ex_id)
                                parent_id = ex_parent_id;
                        }
                    }

                    success = false;

                    if (flags == eNone)
                    {
                        const quint64 ex_flags = query.value(flagsIdx).toUInt(&success);
                        if (success == false)
                            continue;
                        if (ex_flags != eNone)
                            flags = ex_flags;
                    }
                }

                if (parent_id == ex_id)
                    continue;

                if (parent_id == -1)
                    q = QString::fromLatin1("UPDATE SCORES SET PARENT_ID = NULL , FLAGS = ? WHERE NAME = ?");
                else
                    q = QString::fromLatin1("UPDATE SCORES SET PARENT_ID = ? , FLAGS = ? WHERE NAME = ?");

                QSqlQuery subquery(q, db);
                QSqlQuery memoryquery(q, memorydb);
                if (parent_id == -1)
                {
                    subquery.bindValue(0, flags);
                    subquery.bindValue(1, name);
                    memoryquery.bindValue(0, flags);
                    memoryquery.bindValue(1, name);
                }
                else
                {
                    subquery.bindValue(0, parent_id);
                    subquery.bindValue(1, flags);
                    subquery.bindValue(2, name);
                    memoryquery.bindValue(0, parent_id);
                    memoryquery.bindValue(1, flags);
                    memoryquery.bindValue(2, name);
                }
                if (!subquery.exec())
                {
                    qDebug() << subquery.lastError().text() << subquery.lastQuery();
                    continue;
                }

                if (!memoryquery.exec())
                {
                    qDebug() << memoryquery.lastError().text() << memoryquery.lastQuery();
                    continue;
                }

                hasChanges = true;

            } while(query.next());

            if (hasChanges)
                ret = true;
        }
        else
        {
            if (parent_id == -1)
                q = QString::fromLatin1("INSERT INTO SCORES(FLAGS, NAME, ADDED) VALUES (? , ? , ?)");
            else
                q = QString::fromLatin1("INSERT INTO SCORES(PARENT_ID, FLAGS, NAME, ADDED) VALUES (? , ? , ? , ?)");

            QSqlQuery subquery(q, db);
            QSqlQuery memoryquery(q, memorydb);
            const QDate currentDate = QDate::currentDate();
            if (parent_id == -1)
            {
                subquery.bindValue(0, flags);
                subquery.bindValue(1, name);
                subquery.bindValue(2, currentDate);
                memoryquery.bindValue(0, flags);
                memoryquery.bindValue(1, name);
                memoryquery.bindValue(2, currentDate);
            }
            else
            {
                subquery.bindValue(0, parent_id);
                subquery.bindValue(1, flags);
                subquery.bindValue(2, name);
                subquery.bindValue(3, currentDate);
                memoryquery.bindValue(0, parent_id);
                memoryquery.bindValue(1, flags);
                memoryquery.bindValue(2, name);
                memoryquery.bindValue(3, currentDate);
            }
            if (!subquery.exec())
                qDebug() << "main_connection : " << subquery.lastError().text() << subquery.lastQuery();
            else if (!memoryquery.exec())
                qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
            else
                ret = true;
        }

        if (ret)
            for (std::list<SScore>::const_iterator itChild = childs.begin(); itChild != childs.end(); ++itChild)
                ret = ret && setScore(*itChild, replace);
    }
    else
    {
        qDebug() << "cache_connection : " << query.lastError().text() << query.lastQuery();
    }
    db.close();

    return ret;
}

bool DataBase::renameScore(const QString& old_name, const QString& new_name)
{
    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.open(QString(), QString()))
        return false;

    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
    {
        db.close();
        return false;
    }

    const QString q = QString::fromLatin1("UPDATE SCORES SET NAME = ? WHERE NAME = ?");
    QSqlQuery query(q, db);
    QSqlQuery memoryquery(q, memorydb);
    query.bindValue(0, new_name);
    query.bindValue(1, old_name);
    memoryquery.bindValue(0, new_name);
    memoryquery.bindValue(1, old_name);
    bool ret = false;
    if (!query.exec())
        qDebug() << "main_connection : " << query.lastError().text() << query.lastQuery();
    else if (!memoryquery.exec())
        qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
    else
        ret = true;
    db.close();
    return ret;
}

bool DataBase::removeScore(qint64 id)
{
    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.open(QString(), QString()))
        return false;

    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
    {
        db.close();
        return false;
    }

    bool ret = _recRemoveScore(id);
    db.close();
    return ret;
}

std::list<DataBase::STransaction> DataBase::transacts(int year, int month, qint64 score_id)
{
    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
        return std::list<DataBase::STransaction>();

    QString q = QString::fromLatin1("SELECT * FROM TRANSACTIONS");

    const bool hasYear = year >= 0;
    const bool hasMonth = month >=0;
    const bool hasScore = score_id >= 0;

    if (hasYear || hasMonth || hasScore)
        q += QString::fromLatin1(" WHERE ");

    if (hasYear)
    {
        const QString strYear = QStringLiteral("%1").arg(year, 4, 10, QLatin1Char('0'));
        q += QString::fromLatin1("strftime('%Y', DATE) = '") + strYear + QString::fromLatin1("'");
    }

    if (hasYear && (hasMonth || hasScore))
        q += QString::fromLatin1(" AND ");

    if (hasMonth)
    {
        const QString strMonth = QStringLiteral("%1").arg(month, 2, 10, QLatin1Char('0'));
        q += QString::fromLatin1("strftime('%m', DATE) = '") + strMonth + QString::fromLatin1("'");
    }

    if (hasMonth && hasScore)
        q += QString::fromLatin1(" AND ");

    if (hasScore)
        q += QString::fromLatin1("SCORE_ID = ") + QString::number(score_id);

    QSqlQuery query(q, memorydb);
    std::list<DataBase::STransaction> ret;
    if ((query.exec() && query.isValid()) || query.lastError().text().isEmpty())
    {
        const int idIdx = query.record().indexOf("ID");
        const int scoreIdIdx = query.record().indexOf("SCORE_ID");
        const int balanceIdx = query.record().indexOf("BALANCE");
        const int descIdx = query.record().indexOf("DESC");
        const int dateIdx = query.record().indexOf("DATE");
        while (query.next())
        {
            DataBase::STransaction tr;
            bool success = false;
            tr.id = query.value(idIdx).toInt(&success);
            if (success == false)
                continue;

            success = false;
            tr.score_id = query.value(scoreIdIdx).toInt(&success);
            if (success == false)
                continue;

            tr.balance = query.value(balanceIdx).toString();

            tr.desc = query.value(descIdx).toString();

            tr.date = query.value(dateIdx).toDate();

            ret.push_back(tr);
        }
    }
    else
    {
        qDebug() << "cache_connection : " << query.lastError().text() << query.lastQuery();
    }

    return ret;
}

bool DataBase::setTransaction(const STransaction& tr, bool replace)
{
    return setTransaction(tr.date, tr.score_id, tr.balance, tr.desc, replace);
}

bool DataBase::setTransaction(const QDate& date, qint64 score_id, QString balance, QString desc, bool replace)
{
    if (balance.isEmpty() && replace)
        return removeTransaction(date, score_id);

    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.open(QString(), QString()))
        return false;

    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
    {
        db.close();
        return false;
    }

    bool ret = false;
    QString q = QString::fromLatin1("SELECT * FROM TRANSACTIONS WHERE strftime('%Y.%m.%d', DATE) = ? AND SCORE_ID = ?");
    QSqlQuery query(q, memorydb);
    query.bindValue(0, date.toString("yyyy.MM.dd"));
    query.bindValue(1, score_id);
    if ((query.exec() && query.isValid()) || query.lastError().text().isEmpty())
    {
        const int descIdx = query.record().indexOf("DESC");
        const int balanceIdx = query.record().indexOf("BALANCE");
        if (query.next())
        {
            bool hasChanges = false;
            do
            {
                if (!replace)
                {
                    if (balance.isEmpty())
                    {
                        const QString ex_balance = query.value(balanceIdx).toString();
                        if (!ex_balance.isEmpty())
                            balance = ex_balance;
                    }
                    if (desc.isEmpty())
                    {
                        const QString ex_desc = query.value(descIdx).toString();
                        if (!ex_desc.isEmpty())
                            desc = ex_desc;
                    }
                }

                q = QString::fromLatin1("UPDATE TRANSACTIONS SET BALANCE = ? , DESC = ? WHERE strftime('%Y.%m.%d', DATE) = ? AND SCORE_ID = ?");

                QSqlQuery subquery(q, db);
                QSqlQuery memoryquery(q, memorydb);
                const QString formatedDate = date.toString("yyyy.MM.dd");
                subquery.bindValue(0, balance);
                subquery.bindValue(1, desc);
                subquery.bindValue(2, formatedDate);
                subquery.bindValue(3, score_id);
                memoryquery.bindValue(0, balance);
                memoryquery.bindValue(1, desc);
                memoryquery.bindValue(2, formatedDate);
                memoryquery.bindValue(3, score_id);
                if (!subquery.exec())
                {
                    qDebug() << "main_connection : " << subquery.lastError().text() << subquery.lastQuery();
                    continue;
                }

                if (!memoryquery.exec())
                {
                    qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
                    continue;
                }

                hasChanges = true;
            } while (query.next());

            if (hasChanges)
                ret = true;
        }
        else
        {
            q = QString::fromLatin1("INSERT INTO TRANSACTIONS(SCORE_ID, DESC, BALANCE, DATE) VALUES (? , ? , ? , ?)");

            QSqlQuery subquery(q, db);
            QSqlQuery memoryquery(q, memorydb);
            subquery.bindValue(0, score_id);
            subquery.bindValue(1, desc);
            subquery.bindValue(2, balance);
            subquery.bindValue(3, date);
            memoryquery.bindValue(0, score_id);
            memoryquery.bindValue(1, desc);
            memoryquery.bindValue(2, balance);
            memoryquery.bindValue(3, date);

            if (!subquery.exec())
                qDebug() << "main_connection : " << subquery.lastError().text() << subquery.lastQuery();
            else if (!memoryquery.exec())
                qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
            else
                ret = true;
        }
    }
    else
    {
        qDebug() << "cache_connection : " << query.lastError().text() << query.lastQuery();
    }

    db.close();
    return ret;
}

bool DataBase::removeTransaction(qint64 id)
{
    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.open(QString(), QString()))
        return false;

    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
    {
        db.close();
        return false;
    }

    const QString q = QString::fromLatin1("DELETE FROM TRANSACTIONS WHERE ID = ?");
    QSqlQuery query(q, db);
    QSqlQuery memoryquery(q, memorydb);
    query.bindValue(0, id);
    memoryquery.bindValue(0, id);
    bool ret = false;
    if (!query.exec())
        qDebug() << "main_connection : " << query.lastError().text() << query.lastQuery();
    else if (!memoryquery.exec())
        qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
    else
        ret = true;
    db.close();
    return ret;
}

bool DataBase::removeTransaction(const QDate& date, qint64 score_id)
{
    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.open(QString(), QString()))
        return false;

    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
    {
        db.close();
        return false;
    }

    const QString q = QString::fromLatin1("DELETE FROM TRANSACTIONS WHERE strftime('%Y.%m.%d', DATE) = ? AND SCORE_ID = ?");
    QSqlQuery query(q, db);
    QSqlQuery memoryquery(q, memorydb);
    const QString formatedDate = date.toString("yyyy.MM.dd");
    query.bindValue(0, formatedDate);
    query.bindValue(1, score_id);
    memoryquery.bindValue(0, formatedDate);
    memoryquery.bindValue(1, score_id);
    bool ret = false;
    if (!query.exec())
        qDebug() << "main_connection : " << query.lastError().text() << query.lastQuery();
    else if (!memoryquery.exec())
        qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
    else
        ret = true;
    db.close();
    return ret;
}

bool DataBase::moveTransaction(const QDate& old_date, qint64 old_score_id, const QDate& new_date, qint64 new_score_id)
{
    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.open(QString(), QString()))
        return false;

    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
    {
        db.close();
        return false;
    }

    if (old_score_id == new_score_id)
    {
        db.close();
        return true;
    }

    const QString q = QString::fromLatin1("UPDATE TRANSACTIONS SET DATE = ? , SCORE_ID = ? WHERE strftime('%Y.%m.%d', DATE) = ? AND SCORE_ID = ?");
    QSqlQuery query(q, db);
    QSqlQuery memoryquery(q, memorydb);
    const QString formatedOldDate = old_date.toString("yyyy.MM.dd");
    query.bindValue(0, new_date);
    query.bindValue(1, new_score_id);
    query.bindValue(2, formatedOldDate);
    query.bindValue(3, old_score_id);
    memoryquery.bindValue(0, new_date);
    memoryquery.bindValue(1, new_score_id);
    memoryquery.bindValue(2, formatedOldDate);
    memoryquery.bindValue(3, old_score_id);
    bool ret = false;
    if (!query.exec())
        qDebug() << "main_connection : " << query.lastError().text() << query.lastQuery();
    else if (!memoryquery.exec())
        qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
    else
        ret = true;
    db.close();
    return ret;
}

bool DataBase::_recRemoveScore(qint64 id)
{
    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.isOpen())
        return false;

    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
        return false;

    QString q = QString::fromLatin1("SELECT ID FROM TRANSACTIONS WHERE SCORE_ID = ?");
    QSqlQuery query(q, memorydb);
    query.bindValue(0, id);
    if ((query.exec() && query.isValid()) || query.lastError().text().isEmpty())
    {
        q = QString::fromLatin1("SELECT ID FROM SCORES WHERE PARENT_ID = ?");
        QSqlQuery childquery(q, memorydb);
        childquery.bindValue(0, id);
        bool childsRemoved = false;
        if ((childquery.exec() && childquery.isValid()) || childquery.lastError().text().isEmpty())
        {
            if (childquery.next())
            {
                bool success = false;
                const qint64 child_id = childquery.value(childquery.record().indexOf("ID")).toInt(&success);
                if (success)
                    childsRemoved = _recRemoveScore(child_id);
            }
            else
            {
                childsRemoved = true;
            }
        }
        else
        {
            qDebug() << "cache_connection : " << childquery.lastError().text() << childquery.lastQuery();
        }

        if (query.next() || !childsRemoved)
        {
            q = QString::fromLatin1("UPDATE SCORES SET REMOVED = ? WHERE ID = ?");
            QSqlQuery subquery(q, db);
            QSqlQuery memoryquery(q, memorydb);
            const QDate currentDate = QDate::currentDate();
            subquery.bindValue(0, currentDate);
            subquery.bindValue(1, id);
            memoryquery.bindValue(0, currentDate);
            memoryquery.bindValue(1, id);
            if (!subquery.exec())
                qDebug() << "main_connection : " << subquery.lastError().text() << subquery.lastQuery();
            else if (!memoryquery.exec())
                qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
            return false;
        }
        else
        {
            q = QString::fromLatin1("DELETE FROM SCORES WHERE ID = ?");
            QSqlQuery subquery(q, db);
            QSqlQuery memoryquery(q, memorydb);
            subquery.bindValue(0, id);
            memoryquery.bindValue(0, id);
            if (!subquery.exec())
                qDebug() << "main_connection : " << subquery.lastError().text() << subquery.lastQuery();
            else if (!memoryquery.exec())
                qDebug() << "cache_connection : " << memoryquery.lastError().text() << memoryquery.lastQuery();
            else
                return true;
        }
    }
    else
    {
        qDebug() << "cache_connection : " << query.lastError().text() << query.lastQuery();
    }

    return false;
}

void DataBase::_init()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "main_connection");
    db.setDatabaseName("db.sqlite");
    db.setHostName(QString());
    db.setPort(-1);

    if (!db.open(QString(), QString()))
        return;

    _createTables("main_connection");

    _memory_cache();

    db.close();
}

void DataBase::_createTables(const QString& connection)
{
    QSqlDatabase db = QSqlDatabase::database(connection);
    if (!db.isOpen())
        return;

    QSqlQuery q = db.exec("CREATE TABLE IF NOT EXISTS SCORES\
    (\
        ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\
        PARENT_ID INTEGER DEFAULT NULL,\
        FLAGS INT UNSIGNED NOT NULL,\
        NAME TEXT NOT NULL UNIQUE,\
        ADDED DATETIME NOT NULL,\
        REMOVED DATETIME DEFAULT NULL,\
    \
        FOREIGN KEY (PARENT_ID) REFERENCES SCORES (ID) ON DELETE SET NULL\
    );");
    QString err = q.lastError().text();
    if (!err.isEmpty())
        qDebug() << connection << " : " << err << q.lastQuery();
    q = db.exec("CREATE TABLE IF NOT EXISTS TRANSACTIONS\
    (\
        ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\
        SCORE_ID INTEGER DEFAULT NULL,\
        DESC TEXT,\
        BALANCE TEXT,\
        DATE DATETIME NOT NULL,\
    \
        FOREIGN KEY (SCORE_ID) REFERENCES SCORES (ID) ON DELETE SET NULL\
    );");
    err = q.lastError().text();
    if (!err.isEmpty())
        qDebug() << connection << " : " << err << q.lastQuery();
}

void DataBase::_clearMemoryCache()
{
    QSqlDatabase memorydb = QSqlDatabase::database("cache_connection");
    if (!memorydb.isOpen())
        return;

    QStringList tables;
    tables << "SCORES" << "TRANSACTIONS";
    QSqlQuery query(memorydb);
    foreach(const QString& table, tables)
    {
        QSqlQuery q = memorydb.exec(QString("DROP TABLE IF EXISTS %1").arg(table));
        const QString err = q.lastError().text();
        if (!err.isEmpty())
            qDebug() << "cache_connection : " << err << q.lastQuery();
    }
}

QStringList DataBase::_dbDump()
{
    QSqlDatabase db = QSqlDatabase::database("main_connection");
    if (!db.open(QString(), QString()))
        return QStringList();

    QStringList ret;

    QStringList tables;
    tables << "SCORES" << "TRANSACTIONS";

    const QString insert = QString::fromLatin1("INSERT INTO %1 (%2) VALUES (%3);");

    QSqlQuery query(db);
    bool first = true;
    foreach(const QString& table, tables)
    {
        QStringList columns;
        first = true;
        if (!query.exec(QString("SELECT * FROM %1").arg(table)))
            qDebug() << "main_connection : " << query.lastError().text() << query.lastQuery();
        while (query.next())
        {
            QStringList values;
            QSqlRecord record = query.record();
            for (int i = 0; i < record.count(); i++)
            {
                const QString col = record.fieldName(i);
                if (first)
                    columns << col;
                const QVariant val = record.value(i);
                if (!val.isValid() || val.isNull())
                    values << QString::fromLatin1("NULL");
                else
                {
                    if (col == "ADDED" || col == "REMOVED" || col == "DATE")
                        values << (QChar('\'') + val.toDate().toString("yyyy-MM-dd") + QChar('\''));
                    else if (col == "NAME" || col == "DESC" || col == "BALANCE")
                        values << (QChar('\'') + val.toString() + QChar('\''));
                    else
                        values << val.toString();
                }
            }
            first = false;

            ret.push_back(insert.arg(table).arg(columns.join(", ")).arg(values.join(", ")));
        }

        query.clear();
    }

    db.close();

    return ret;
}

void DataBase::_memory_cache()
{
    QSqlDatabase memorydb = QSqlDatabase::addDatabase("QSQLITE", "cache_connection");
    memorydb.setDatabaseName(":memory:");
    memorydb.setHostName(QString());
    memorydb.setPort(-1);

    if (!memorydb.open(QString(), QString()))
        return;

    //_clearMemoryCache();

    _createTables("cache_connection");

    const QStringList dump = _dbDump();
    if (dump.isEmpty())
        return;

    QSqlQuery query(memorydb);

    query.exec("BEGIN;");
    foreach(const QString& q, dump)
        if (!query.exec(q))
            qDebug() << "cache_connection : " << query.lastError().text() << q;
    if (!query.exec("COMMIT;"))
            qDebug() << "cache_connection : " << query.lastError().text() << query.lastQuery();
}
