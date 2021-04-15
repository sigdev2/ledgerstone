#include "actions.h"

QString Actions::m_separators = QLatin1String("|:+<");

Actions* Actions::instance()
{
    static Actions acts;
    return &acts;
}

Actions::Actions()
{
}

QAction* Actions::get(TActionString id, bool createIfNo)
{
    const QString strId = QString::fromStdString(id);
    if (createIfNo && !m_actions.contains(strId))
        if (add(id) == NULL)
            return NULL;

    return m_actions[strId];
}

Actions* Actions::add(TActionString id, QAction* action)
{
    bool isCheckable = false;
    std::string idName = id;
    if (idName.at(0) == '+')
    {
        isCheckable = true;
        idName.erase(0, 1);
    }

    if (idName.empty() || !_checkSeporators(idName))
        return NULL;

    const QString strId = QString::fromStdString(idName);
    if (m_actions.contains(strId))
        return NULL;

    action->setParent(this);
    if (!action->isCheckable() && isCheckable)
        action->setCheckable(isCheckable);
    m_actions.insert(strId, action);
    return this;
}

Actions* Actions::add(TActionString id, TActionString text) { return add(id, new QAction(tr(text.c_str()), this)); }
Actions* Actions::add(TActionString id, const QIcon& icon, TActionString text) { return add(id, new QAction(icon, tr(text.c_str()), this)); }
Actions* Actions::add(QAction* action) { return add(action->text().toStdString(), action); }
Actions* Actions::add(TActionString text) { return add(text, text); }
Actions* Actions::add(const QIcon& icon, TActionString text) { return add(text, icon, text); }

QActionGroup* Actions::getGroup(TActionString id, bool createIfNo)
{
    const QString strId = QString::fromStdString(id);
    if (createIfNo && !m_groups.contains(strId))
        if (addGroup(id) == NULL)
            return NULL;

    return m_groups[strId];
}

Actions* Actions::addGroup(TActionString id)
{
    const QString strId = QString::fromStdString(id);
    if (id.empty() || !m_groups.contains(strId))
        return NULL;

    QActionGroup* newGroup = new QActionGroup(this);
    m_groups.insert(strId, newGroup);

    return this;
}

Actions* Actions::addGroup(TActionString id, TActionString actions)
{
    QActionGroup* newGroup = getGroup(id, true);

    QStringList actionsList = QString::fromStdString(actions).split(QRegExp("\\s+"));
    for (QStringList::iterator it = actionsList.begin(); it != actionsList.end(); ++it)
    {
        QAction* action = get(it->toStdString(), true);
        if (action != NULL)
            newGroup->addAction(action);
    }

    return this;
}

bool Actions::_checkSeporators(TActionString text)
{
    const QString strText = QString::fromStdString(text);
    const QRegExp re("\\s");
    for (QString::iterator it = m_separators.begin(); it != m_separators.end(); ++it)
        if (strText.contains(*it) || re.indexIn(*it) != -1)
            return false;

    return true;
}
