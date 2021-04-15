#ifndef CALCULATEENGINE_H
#define CALCULATEENGINE_H

#include <QScriptEngine>

#include <unordered_map>

class MonthModel;

class EngineInitializer : public QScriptEngine
{
public:
    EngineInitializer();
    virtual ~EngineInitializer() {};
};

class CalculateEngine
{
public:
    CalculateEngine() {};
    virtual ~CalculateEngine() {};

    QString calculate(const QString& expression) const;

    void setMonthModel(int y, int n, MonthModel* m) const;
    void removeModel(int y, int n) const;

private:
    static EngineInitializer& _engine();
};

#endif // CALCULATEENGINE_H
