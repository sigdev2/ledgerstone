#include "theme.h"

Theme::Theme(const QString& name)
   : m_name(name)
{
    m_colors.reserve(eRolesCount);
    for (int i = 0; i < eRolesCount; ++i)
        m_colors.push_back(TColorMap());
    init();
}

Theme& Theme::setColor(ERole role, quint64 id, const QColor& color)
{
    if (role == eRolesCount)
        return *this;

    m_colors[static_cast<int>(role)][id] = color;
    return *this;
}

QColor Theme::color(ERole role, quint64 id, const QString s) const
{
    if (role == eRolesCount)
        return QColor();
    const TColorMap& map = m_colors[static_cast<int>(role)];
    TCItColorMap it = map.find(id);
    if (it == map.end())
        return QColor();

    return it->second;
}
