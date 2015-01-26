#ifndef FILECONSUMER_H
#define FILECONSUMER_H

#include <QCoreApplication>
#include <QScriptEngine>
#include <functional>

#include "message.h"

class FileConsumer : public QObject
{
    Q_OBJECT

    QString path;
    QString archPath;
    QString errorPath;

    std::function<QString()> archPathFunc;

    bool consuming;
    bool processing;

    long procceded;
    long commited;
public:
    FileConsumer();
    bool valid(const QString& filename);

    QString getPath() const;

    static bool initScriptEngine(QScriptEngine &engine);

    QString getArchPath() const;

    QString getErrorPath() const;

    std::function<QString ()> getArchPathFunc() const;

signals:
    void message(Message msg);

public slots:

    void consume();
    void consume(const QString& str);

    void commit(Message msg);

    void rollback(Message msg);
    void setPath(const QString &value);
    void setArchPath(const QString &value);
    void setErrorPath(const QString &value);
    void setArchPathFunc(const std::function<QString ()> &value);
};

#endif // FILECONSUMER_H
