#ifndef FILECONSUMER_H
#define FILECONSUMER_H

#include <QCoreApplication>
#include <QScriptEngine>
#include <QSharedPointer>

#include "message.h"
#include "common.h"

/**
 * @brief The FileConsumerCommiter class
 * Multithreaded commiter
 * FIXME Draft.
 */
class FileConsumerCommiter : public QObject
{
    Q_OBJECT

    QString archPath;
    QString errorPath;

    STD_FUNCTION<QString()> archPathFunc;

public:
    explicit FileConsumerCommiter(QObject *parent = 0);
    static bool initScriptEngine(QScriptEngine &engine);

    QString getArchPath() const;

signals:
    void commited(PMessage msg);
    void rollbacked(PMessage msg);

public slots:
    void commit(PMessage msg);
    void rollback(PMessage msg);
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

    bool isquit;

public:
    explicit FileConsumer(QObject *parent = 0);
    ~FileConsumer();
    bool valid(const QString& filename);

    QString getPath() const;

    static bool initScriptEngine(QScriptEngine &engine);

    QString getArchPath() const;

    QString getErrorPath() const;

    STD_FUNCTION<QString ()> getArchPathFunc() const;
    QString callArchPathFuncGlobalScript();

    int getBatchSize() const;

    static void messageDeleter(Message *msg);

signals:
    void message(PMessage msg);

public slots:
    int init();

    void consume();
    void consume(const QString& str);

    void commit(PMessage msg);
    void rollback(PMessage msg);

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

#endif // FILECONSUMER_H
