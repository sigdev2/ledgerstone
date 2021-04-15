#ifndef ACTIONS_H
#define ACTIONS_H

#include <QHash>
#include <QObject>
#include <QActionGroup>

#include <assert.h>

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

#include <unordered_map>

class QAction;

typedef const std::string& TActionString;

class Actions : public QObject
{
public:
    static Actions* instance();

    Actions();

    QAction*      get(TActionString id, bool createIfNo = true);

    Actions* add(QAction* action);
    Actions* add(TActionString text);
    Actions* add(const QIcon& icon, TActionString text);
    Actions* add(TActionString id, QAction* action);
    Actions* add(TActionString id, TActionString text);
    Actions* add(TActionString id, const QIcon& icon, TActionString text);

    QActionGroup* getGroup(TActionString id, bool createIfNo = true);

    Actions* addGroup(TActionString id);
    Actions* addGroup(TActionString id, TActionString actions);

private:
    static QString m_separators;

    bool _checkSeporators(TActionString text);

    QHash<QString, QAction*> m_actions;
    QHash<QString, QActionGroup*> m_groups;
};

template<class TBase>
class QActionWidget
{
public:
    QActionWidget(TBase* _parent) : parent(_parent) {}
    TBase* parent;

    QAction* addAction(TActionString text);
    QAction* addAction(const QIcon& icon, TActionString text);
    QAction* addAction(TActionString text, const QObject* receiver, const char* member, const QKeySequence& shortcut = QKeySequence());
    QAction* addAction(const QIcon& icon, TActionString text, const QObject* receiver, const char* member, const QKeySequence& shortcut = QKeySequence());

    QAction* addAction(TActionString id, TActionString text);
    QAction* addAction(TActionString id, const QIcon& icon, TActionString text);
    QAction* addAction(TActionString id, TActionString text, const QObject* receiver, const char* member, const QKeySequence& shortcut = QKeySequence());
    QAction* addAction(TActionString id, const QIcon& icon, TActionString text, const QObject* receiver, const char* member, const QKeySequence& shortcut = QKeySequence());

    void	 addAction(QAction* action);
};

template <class TBase>
QAction* QActionWidget<TBase>::addAction(TActionString text)
{
    QAction* newAct = Actions::instance()->get(text);
    assert(newAct != NULL);
    if (newAct != NULL)
        parent->addAction(newAct);

    return newAct;
}

template <class TBase>
QAction* QActionWidget<TBase>::addAction(const QIcon& icon, TActionString text)
{
    QAction* newAct = addAction(text);
    if (newAct == NULL)
        return newAct;

    newAct->setIcon(icon);
    return newAct;
}

template <class TBase>
QAction* QActionWidget<TBase>::addAction(TActionString text, const QObject* receiver, const char* member, const QKeySequence& shortcut)
{
    QAction* newAct = addAction(text);
    if (newAct != NULL)
    {
        if (!shortcut.isEmpty()) newAct->setShortcut(shortcut);
        connect(newAct, SIGNAL(triggered()), receiver, member);
    }

    return newAct;
}

template <class TBase>
QAction* QActionWidget<TBase>::addAction(const QIcon& icon, TActionString text, const QObject* receiver, const char* member, const QKeySequence& shortcut)
{
    QAction* newAct = addAction(icon, text);
    if (newAct != NULL)
    {
        if (!shortcut.isEmpty()) newAct->setShortcut(shortcut);
        connect(newAct, SIGNAL(triggered()), receiver, member);
    }

    return newAct;
}

template <class TBase>
QAction* QActionWidget<TBase>::addAction(TActionString id, TActionString text)
{
    QAction* newAct = addAction(id);
    if (newAct != NULL)
        newAct->setText(text);

    return newAct;
}

template <class TBase>
QAction* QActionWidget<TBase>::addAction(TActionString id, const QIcon& icon, TActionString text)
{
    QAction* newAct = addAction(id, text);
    if (newAct == NULL)
        return newAct;

    newAct->setIcon(icon);
    return newAct;
}

template <class TBase>
QAction* QActionWidget<TBase>::addAction(TActionString id, TActionString text, const QObject* receiver, const char* member, const QKeySequence& shortcut)
{
    QAction* newAct = addAction(id, text);
    if (newAct != NULL)
    {
        if (!shortcut.isEmpty()) newAct->setShortcut(shortcut);
        connect(newAct, SIGNAL(triggered()), receiver, member);
    }

    return newAct;
}

template <class TBase>
QAction* QActionWidget<TBase>::addAction(TActionString id, const QIcon& icon, TActionString text, const QObject* receiver, const char* member, const QKeySequence& shortcut)
{
    QAction* newAct = addAction(id, icon, text);
    if (newAct != NULL)
    {
        if (!shortcut.isEmpty()) newAct->setShortcut(shortcut);
        connect(newAct, SIGNAL(triggered()), receiver, member);
    }

    return newAct;
}

template <class TBase>
void QActionWidget<TBase>::addAction(QAction* action)
{
    Actions* inst = Actions::instance();
    if (inst->add(action) == NULL)
    {
        assert(0);
        return;
    }

    parent->addAction(action);
}

class QActionMenu : public QActionWidget<QMenu>
{
public:
    QActionMenu(QMenu* parent) : QActionWidget<QMenu>(parent) {}

    QActionMenu* addActions(TActionString _str)
    {
        QString str = QString::fromStdString(_str);
        if (str.isEmpty())
            return NULL;

        str = str.replace('|', QLatin1String(" | "));
        str = str.replace('<', QLatin1String(" < "));

        QStringList list = str.split(QRegExp("\\s+"));

        if (list.isEmpty())
            return NULL;

        QList<QMenu*> root;
        root.push_back(parent);
        for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
        {
            if (it->isEmpty())
                continue;

            if (*it == QLatin1String("|"))
            {
                root.back()->addSeparator();
            }
            else if (it->right(1) == QLatin1String(":"))
            {
                it->chop(1);
                root.push_back(root.back()->addMenu(*it));
            }
            else if (*it == QLatin1String("<"))
            {
                root.pop_back();
            }
            else
            {
                QString name = *it;
                bool isCheckable = false;
                if (name.left(1) == QLatin1String("+"))
                {
                    isCheckable = true;
                    name = name.mid(1);
                }

                QAction* newAct = Actions::instance()->get(name.toStdString());
                if (newAct == NULL)
                    continue;

                root.back()->addAction(newAct);
                newAct->setCheckable(isCheckable);
            }
        }

        return this;
    }
};

class QActionMenuBar : public QActionWidget<QMenuBar>
{
public:
    QActionMenuBar(QMenuBar* parent) : QActionWidget<QMenuBar>(parent) {}

    QActionMenuBar* addActions(TActionString _str)
    {
        QString str = QString::fromStdString(_str);
        if (str.isEmpty())
            return NULL;

        str = str.replace('|', QLatin1String(" | "));
        str = str.replace('<', QLatin1String(" < "));

        QStringList list = str.split(QRegExp("\\s+"));

        if (list.isEmpty())
            return NULL;

        QList<QMenu*> root;
        for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
        {
            if (it->isEmpty())
                continue;

            QString name = *it;
            if (name == QLatin1String("|") || name == QLatin1String("<"))
            {
                if (root.isEmpty())
                    continue;

                if (name == QLatin1String("|"))
                    root.back()->addSeparator();
                else // is <
                    root.pop_back();
            }
            else
            {
                if (name.right(1) == QLatin1String(":"))
                {
                    name.chop(1);
                    QMenu* m = root.isEmpty() ? parent->addMenu(name) : root.back()->addMenu(name);
                    root.push_back(m);
                    m_submenus[name] = m;
                }
                else
                {
                    bool isCheckable = false;
                    if (name.left(1) == QLatin1String("+"))
                    {
                        isCheckable = true;
                        name = name.mid(1);
                    }

                    QAction* newAct = Actions::instance()->get(name.toStdString());
                    if (newAct == NULL)
                        continue;

                    if (root.isEmpty())
                        parent->addAction(newAct);
                    else
                        root.back()->addAction(newAct);

                    if (!newAct->isCheckable() && isCheckable)
                        newAct->setCheckable(isCheckable);
                }
            }
        }

        return this;
    }

    QMenu* getMenu(TActionString _str)
    {
        QString str = QString::fromStdString(_str);
        if (str.isEmpty())
            return NULL;
        std::unordered_map<QString, QMenu*>::iterator it = m_submenus.find(str);
        if (it == m_submenus.end())
            return NULL;
        return it->second;
    }

private:
    std::unordered_map<QString, QMenu*> m_submenus;
};

class QActionToolBar : public QActionWidget<QToolBar>
{
public:
    QActionToolBar(QToolBar* parent) : QActionWidget<QToolBar>(parent) {}

    QActionToolBar* addActions(TActionString _str)
    {
        QString str = QString::fromStdString(_str);
        if (str.isEmpty())
            return NULL;

        QStringList list = str.split(QRegExp("\\s+"));

        if (list.isEmpty())
            return NULL;

        for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
        {
            QString name = *it;
            bool isCheckable = false;
            if (name.left(1) == QLatin1String("+"))
            {
                isCheckable = true;
                name = name.mid(1);
            }

            QAction* newAct = Actions::instance()->get(name.toStdString());
            if (newAct == NULL)
                continue;

            parent->addAction(newAct);
            newAct->setCheckable(isCheckable);
        }

        return this;
    }
};

/* example
 *
 * MainMenu: File: Open New | Sasve SaveAs | Close <
 *           View: Show Hide | +Folders <
 *
 */

#endif // ACTIONS_H
