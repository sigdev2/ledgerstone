#ifndef DEFAULTTHEME_H
#define DEFAULTTHEME_H

#include "theme.h"

class DefaultTheme : public Theme
{
public:
    enum EAdditionalRoles
    {
        eRemoved = 0x1000
    };

    DefaultTheme();

    virtual DefaultTheme& setColor(ERole role, quint64 id, const QColor& color) { Q_UNUSED(role);Q_UNUSED(id);Q_UNUSED(color); return *this; };
    virtual QColor color(ERole role, quint64 id, const QString s = QString()) const;

protected:
    virtual void init();
};

#endif // DEFAULTTHEME_H
