#ifndef WMQPRODUCER_H
#define WMQPRODUCER_H

#include <QCoreApplication>
#include <QScriptEngine>
#include "wmqconnection.h"
#include "message.h"

QByteArray* buildMQRFHeader2(Message *msg);

class WMQProducerCommiter : public QObject
{
    Q_OBJECT
public:
    static bool initScriptEngine(QScriptEngine &engine);

signals:
    void commited(Message *msg);
    void rollbacked(Message *msg);
public slots:
    void commit(Message *msg);
    void rollback(Message *msg);

};

class WMQProducerThread : public QObject
{
    Q_OBJECT

    iConnectionFactory* connectionFactory;
    iConnection* connection;
    QString queueName;
    ImqQueue queue;

    bool inuse;

    int workerNumber;

    QReadWriteLock lock;

public:
    WMQProducerThread(iConnectionFactory* _connectionFactory);
    ~WMQProducerThread();


    bool doSend(Message *msg);

    QString getQueueName() const;

signals:
    void sended(Message *msg);
    void got(Message *msg);
    void error(Message *message, QString err);

public slots:
    void send(Message *message);
    void setQueueName(const QString &value);
    void setWorkerNumber(int n);
};


class WMQProducer : public QObject
{
    Q_OBJECT

    iConnectionFactory* connectionFactory;
    QString queueName;

    QList<QThread*> threads;
    QList<WMQProducerThread*> workers;

    QThread *commiterThread;
    WMQProducerCommiter *commiter;

    int maxWorkers;

    int workerCounter;

public:
    WMQProducer();
    WMQProducer(iConnectionFactory* connectionFactory);
    ~WMQProducer();

    static bool initScriptEngine(QScriptEngine &engine);

    int nextRoundRobbin() {if (++workerCounter >= workers.size()) workerCounter = 0; return workerCounter; }

    QString getQueueName() const;

    int getMaxWorkers() const;

    iConnectionFactory *getConnectionFactory() const;

signals:
    void produced(Message *msg);
    void error(Message *message, QString err);
    void rollback(Message *message);

public slots:
    void produce(Message *msg);
    void workerProduced(Message *msg);
    void getError(Message *message, QString err);
    void setQueueName(const QString &value);
    void setMaxWorkers(int value);
    void setConnectionFactory(iConnectionFactory *value);
    void init();

    QObject *getCommiter();
};

#endif // WMQPRODUCER_H
