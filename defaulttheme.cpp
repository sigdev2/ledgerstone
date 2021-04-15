#include "defaulttheme.h"
#include "database.h"
#include "monthmodel.h"

DefaultTheme::DefaultTheme() : Theme("Default")
{

}

QColor DefaultTheme::color(ERole role, quint64 id, const QString s) const
{
    // default
    QColor background(240, 240, 240);
    QColor font(51, 51, 51);

    if (role == eFont)
    {
        if (s.startsWith("-"))
            font = QColor(250, 107, 107);
        else if ((id & MonthModel::signedScoreTypes()) != 0 && s != QString::fromLatin1("0.00"))
            font = QColor(76, 186, 96);
        else if ((id & (MonthModel::eColResult | MonthModel::eResult | MonthModel::eRowResult)) != 0)
            font = QColor(Qt::black);
        return font;
    }
    else if (role == eBackground)
    {
        if (id & eRemoved)
        {
            if (id & MonthModel::eColResult)
                background = QColor(255, 227, 227);
            else if (id & MonthModel::eHeader)
                background = QColor(255, 196, 196);
            else
                background = QColor(255, 240, 240);
        }
        else
        {
            if (id & MonthModel::eResult)
                background = QColor(241, 255, 227);
            else if (id & MonthModel::eColResult)
            {
                if ((id & MonthModel::сumulativeScoreTypes()) != 0)
                    background = QColor(235, 222, 255);
                else if ((id & MonthModel::allScoreTypes()) == 0)
                    background = QColor(237, 237, 237);
                else
                    background = QColor(219, 239, 255);
            }
            else if (id & MonthModel::eRowResult)
            {
                background = QColor(219, 239, 255);
            }
            else if (id & MonthModel::eSplitRow)
            {
                background = QColor(Qt::white);
            }
            else if (id & MonthModel::eHeader)
            {
                background = QColor(255, 250, 227);
            }
            else if (id & MonthModel::eCell)
            {
                if ((id & MonthModel::сumulativeScoreTypes()) != 0)
                    background = QColor(244, 237, 255);
                else if ((id & MonthModel::allScoreTypes()) == 0)
                    background = QColor(247, 247, 247);
                else
                    background = QColor(Qt::white);
            }
        }

        return background;
    }

    return QColor();
}

void DefaultTheme::init()
{

}
