#include "calculateengine.h"

#include <QDebug>

#include "monthmodel.h"

EngineInitializer::EngineInitializer()
{
    /*evaluate(QString::fromLatin1("function pad(num, size)"
                                 "{"
                                 "    num = num.toString();"
                                 "    while (num.length < size) num = '0' + num;"
                                 "    return num;"
                                 "}"));*/

    evaluate(QString::fromLatin1("function month(year, month)"
                                 "{"
                                 "    return data[year][month]"
                                 "}"));
}

QString CalculateEngine::calculate(const QString& expression) const
{
    if (expression.isEmpty())
        return expression;

    // calculating

    QScriptValue result = _engine().evaluate(QString::fromLatin1("Number(%1).toFixed(2)").arg(expression));
    if (_engine().hasUncaughtException())
    {
        int line = _engine().uncaughtExceptionLineNumber();
        qDebug() << "Uncaught exception at line" << line << ":" << result.toString();
        return QString::fromLatin1("ERROR");
    }

    const QString ret = result.toString();
    if (ret == QString::fromLatin1("undefined"))
    {
        qDebug() << "undefined value for expression " << expression;
        return QString();
    }

    return ret;
}

void CalculateEngine::setMonthModel(int y, int n, MonthModel* m) const
{
    QScriptEngine& e = _engine();
    QScriptValue data = e.globalObject().property("data");
    if (!data.isValid())
    {
        data = e.newObject();
        e.globalObject().setProperty("data", data);
    }

    QScriptValue year = data.property(y);
    if (!year.isValid())
    {
        year = e.newObject();
        data.setProperty(y, year);
    }

    year.setProperty(n, e.newQObject(m));
}

void CalculateEngine::removeModel(int y, int n) const
{
    QScriptValue data = _engine().globalObject().property("data");
    if (!data.isValid())
        return;

    QScriptValue year = data.property(y);
    if (!year.isValid())
        return;

    QScriptValue month = year.property(n);
    if (!month.isValid())
        return;

    year.setProperty(n, QScriptValue());
}

EngineInitializer& CalculateEngine::_engine()
{
    static EngineInitializer engine;
    return engine;
}
