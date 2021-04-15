#ifndef THEME_H
#define THEME_H

#include <unordered_map>

#include <QString>
#include <QColor>

class Theme
{
public:
    enum ERole
    {
        eBackground = 0,
        eFont,

        eRolesCount
    };

    Theme(const QString& name = QString());

    Theme& setName(const QString& name) { m_name = name; return *this; }
    QString name() const { return m_name; }

    virtual Theme& setColor(ERole role, quint64 id, const QColor& color);
    virtual QColor color(ERole role, quint64 id, const QString s = QString()) const;

protected:
    typedef std::unordered_map<quint64, QColor> TColorMap;
    typedef TColorMap::iterator TItColorMap;
    typedef TColorMap::const_iterator TCItColorMap;

    typedef std::vector<TColorMap> TColors;
    typedef TColors::iterator TItColors;
    typedef TColors::const_iterator TCItColors;

    virtual void init() {};

private:
    QString m_name;

    std::vector<std::unordered_map<quint64, QColor> > m_colors;
};

#endif // THEME_H
