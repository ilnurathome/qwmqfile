#ifndef TFILECONSUMER_H
#define TFILECONSUMER_H

#include <QCoreApplication>
#include <QScriptEngine>

#include "message.h"
#include "common.h"

class TFileConsumerCommiter : public QObject
{
    Q_OBJECT

    QString archPath;
    QString errorPath;

    STD_FUNCTION<QString()> archPathFunc;

public:
    explicit TFileConsumerCommiter(QObject *parent = 0);
    static bool initScriptEngine(QScriptEngine &engine);

    QString getArchPath() const;

signals:
    void commited(TMessage<QFile> *msg);
    void rollbacked(TMessage<QFile> *msg);
public slots:
    void commit(TMessage<QFile> *msg);
    void rollback(TMessage<QFile> *msg);
    void setArchPath(const QString &value);
    void setArchPathFunc(const STD_FUNCTION<QString ()> &value);
};

class TFileConsumer : public QObject
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
    TFileConsumerCommiter *commiter;

    bool isquit;

public:
    explicit TFileConsumer(QObject *parent = 0);
    ~TFileConsumer();
    bool valid(const QString& filename);

    QString getPath() const;

    static bool initScriptEngine(QScriptEngine &engine);

    QString getArchPath() const;

    QString getErrorPath() const;

    STD_FUNCTION<QString ()> getArchPathFunc() const;
    QString callArchPathFuncGlobalScript();

    int getBatchSize() const;

signals:
    void message(TMessage<QFile> *msg);

public slots:
    int init();

    void consume();
    void consume(const QString& str);

    void commit(TMessage<QFile> *msg);
    void rollback(TMessage<QFile> *msg);

    void setBatchSize(int value);

    void setPath(const QString &value);
    void setArchPath(const QString &value);
    void setErrorPath(const QString &value);
    void setArchPathFunc(const STD_FUNCTION<QString ()> &value);
    void setArchPathFuncGlobal(const QString &value);

    QObject *getCommiter();
    void moveToThread(QObject *thread);

    void quit();
};

#endif // TFILECONSUMER_H
