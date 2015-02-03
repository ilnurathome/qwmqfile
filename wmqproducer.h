#ifndef WMQPRODUCER_H
#define WMQPRODUCER_H

#include <QCoreApplication>
#include <QScriptEngine>
#include "wmqconnection.h"
#include "message.h"

QByteArray* buildMQRFHeader2(QSharedPointer<Message>msg);

class WMQProducerCommiter : public QObject
{
    Q_OBJECT
public:
    static bool initScriptEngine(QScriptEngine &engine);

signals:
    void commited(QSharedPointer<Message>msg);
    void rollbacked(QSharedPointer<Message>msg);
public slots:
    void commit(QSharedPointer<Message>msg);
    void rollback(QSharedPointer<Message>msg);

};

class WMQProducer : public QObject
{
    Q_OBJECT

    iConnectionFactory* connectionFactory;
    iConnection* connection;

    bool inuse;

    int workerNumber;

    QString queueName;
    ImqQueue queue;

    QReadWriteLock lock;

public:
    WMQProducer();
    WMQProducer(iConnectionFactory* _connectionFactory);
    ~WMQProducer();

    static bool initScriptEngine(QScriptEngine &engine);

    bool doSend(QSharedPointer<Message>msg);

    QString getQueueName() const;

    iConnectionFactory *getConnectionFactory() const;

signals:
    void produced(QSharedPointer<Message>msg);
    void got(QSharedPointer<Message>msg);
    void error(QSharedPointer<Message>message, QString err);
    void rollback(QSharedPointer<Message>msg);

public slots:
    void produce(QSharedPointer<Message>message);
    void setQueueName(const QString &value);
    void setWorkerNumber(int n);

    void setConnectionFactory(iConnectionFactory *value);
};


class WMQProducerThreaded : public QObject
{
    Q_OBJECT

    iConnectionFactory* connectionFactory;
    QString queueName;

    QList<QThread*> threads;
    QList<WMQProducer*> workers;

    QThread *commiterThread;
    WMQProducerCommiter *commiter;

    int maxWorkers;

    int workerCounter;

public:
    WMQProducerThreaded(iConnectionFactory* connectionFactory=0);
    ~WMQProducerThreaded();

    static bool initScriptEngine(QScriptEngine &engine);

    int nextRoundRobbin() {if (++workerCounter >= workers.size()) workerCounter = 0; return workerCounter; }

    QString getQueueName() const;

    int getMaxWorkers() const;

    iConnectionFactory *getConnectionFactory() const;

signals:
    void produced(QSharedPointer<Message>msg);
    void error(QSharedPointer<Message>message, QString err);
    void rollback(QSharedPointer<Message>message);

public slots:
    void produce(QSharedPointer<Message>msg);
    void workerProduced(QSharedPointer<Message>msg);
    void getError(QSharedPointer<Message>message, QString err);
    void setQueueName(const QString &value);
    void setMaxWorkers(int value);
    void setConnectionFactory(iConnectionFactory *value);
    int init();

    QObject *getCommiter();
};

#endif // WMQPRODUCER_H
