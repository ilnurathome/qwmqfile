#ifndef FILECONSUMER_H
#define FILECONSUMER_H

#include <QCoreApplication>
#include <QScriptEngine>

#include "message.h"
#include "common.h"

class FileConsumerCommiter : public QObject
{
    Q_OBJECT

    QString archPath;
    QString errorPath;

    STD_FUNCTION<QString()> archPathFunc;

public:
    static bool initScriptEngine(QScriptEngine &engine);

    QString getArchPath() const;

signals:
    void commited(Message msg);
    void rollbacked(Message msg);
public slots:
    void commit(Message msg);
    void rollback(Message msg);
    void setArchPath(const QString &value);
    void setArchPathFunc(const STD_FUNCTION<QString ()> &value);
};

class FileConsumer : public QObject
{
    Q_OBJECT

    QString path;
    QString archPath;
    QString errorPath;

    int batchSize;

    STD_FUNCTION<QString()> archPathFunc;
    QScriptValue archPathFuncScript;

    bool consuming;
    bool processing;

    long procceded;
    long commited;

    QThread *commiterThread;
    FileConsumerCommiter *commiter;

public:
    FileConsumer();
    ~FileConsumer();
    bool valid(const QString& filename);

    QString getPath() const;

    static bool initScriptEngine(QScriptEngine &engine);

    QString getArchPath() const;

    QString getErrorPath() const;

    STD_FUNCTION<QString ()> getArchPathFunc() const;
    QString callArchPathFuncGlobalScript();

    int getBatchSize() const;

signals:
    void message(Message msg);

public slots:
    void init();

    void consume();
    void consume(const QString& str);

    void commit(Message msg);
    void rollback(Message msg);

    void commitAndMove(Message msg);
    void rollbackAndMove(Message msg);

    void setBatchSize(int value);

    void setPath(const QString &value);
    void setArchPath(const QString &value);
    void setErrorPath(const QString &value);
    void setArchPathFunc(const STD_FUNCTION<QString ()> &value);
    void setArchPathFuncGlobal(const QString &value);

    QObject *getCommiter();
    void moveToThread(QObject *thread);
};

#endif // FILECONSUMER_H
