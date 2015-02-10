#ifndef WMQPRODUCER_H
#define WMQPRODUCER_H

#include <QSemaphore>
#include <QCoreApplication>
#include <QScriptEngine>
#include "wmqconnection.h"
#include "message.h"

QByteArray* buildMQRFHeader2(PMessage msg);

class WMQProducerCommiter : public QObject
{
    Q_OBJECT
public:
    static bool initScriptEngine(QScriptEngine &engine);

signals:
    void commited(PMessage msg);
    void rollbacked(PMessage msg);
public slots:
    void commit(PMessage msg);
    void rollback(PMessage msg);

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

    QSemaphore *semaphore;

public:
    WMQProducer();
    WMQProducer(iConnectionFactory* _connectionFactory);
    ~WMQProducer();

    static bool initScriptEngine(QScriptEngine &engine);

    bool doSend(PMessage msg);
    bool process(PMessage msg);

    QString getQueueName() const;

    iConnectionFactory *getConnectionFactory() const;

    QSemaphore *getSemaphore() const;
    void setSemaphore(QSemaphore *value);

signals:
    void produced(PMessage msg);
    void got(PMessage msg);
    void error(PMessage message, QString err);
    void rollback(PMessage msg);

public slots:
    void produce(PMessage message);
    void setQueueName(const QString &value);
    void setWorkerNumber(int n);

    void setConnectionFactory(iConnectionFactory *value);
};

/**
 * @brief The WMQProducerThreaded class
 * Producer in multithreaded model auto create several threads and move object instance of WMQProducer to.
 *
 *                                       +-<<QueuedConn>>- WMQProducer --+
 *                                       |                               |
 * one consumer -- WMQProducerThreaded --+-<<QueuedConn>>- WMQProducer --+-<<QueuedConn>>- WMQProducerCommiter
 *                                       |                               |
 *                                       +-<<QueuedConn>>- WMQProducer --+
 */
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

    QSemaphore *semaphore;

public:
    WMQProducerThreaded(iConnectionFactory* connectionFactory=0);
    ~WMQProducerThreaded();

    static bool initScriptEngine(QScriptEngine &engine);

    int nextRoundRobbin() {if (++workerCounter >= workers.size()) workerCounter = 0; return workerCounter; }

    QString getQueueName() const;

    int getMaxWorkers() const;

    iConnectionFactory *getConnectionFactory() const;

signals:
    void produced(PMessage msg);
    void error(PMessage message, QString err);
    void rollback(PMessage message);

public slots:
    void produce(PMessage msg);
    void workerProduced(PMessage msg);
    void getError(PMessage message, QString err);
    void setQueueName(const QString &value);
    void setMaxWorkers(int value);
    void setConnectionFactory(iConnectionFactory *value);
    int init();

    QObject *getCommiter();
};

#endif // WMQPRODUCER_H
