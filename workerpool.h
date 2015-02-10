#ifndef WORKERPOOL_H
#define WORKERPOOL_H

#include <QThread>
#include <QScriptEngine>
#include <QSemaphore>
#include <QReadWriteLock>
#include <QObject>
#include "message.h"

class WorkerCommiter : public QObject
{
    Q_OBJECT
public:
    static bool initScriptEngine(QScriptEngine &engine);

signals:
    void commited(PMessage msg);
    void rollbacked(PMessage msg);
public slots:
    void commit(PMessage msg) {emit commited(msg);}
    void rollback(PMessage msg) {emit rollbacked(msg);}
};

class WorkerProxy : public QObject
{
    Q_OBJECT

    bool inuse;

    QReadWriteLock lock;

    QSemaphore *semaphore;

    QObject *worker;

public:
    explicit WorkerProxy(QObject *parent = 0);

    QSemaphore *getSemaphore() const;
    void setSemaphore(QSemaphore *value);

    QObject *getWorker() const;
    void setWorker(QObject *value);

signals:
    void produced(PMessage msg);
    void rollbacked(PMessage msg);
    void got(PMessage msg);
    void send(PMessage msg);

public slots:
    bool produce(PMessage msg);
    void proccess(PMessage msg);
    void commit(PMessage msg);
    void rollback(PMessage msg);
};

class WorkerPool : public QObject
{
    Q_OBJECT

    QList<QThread*> threads;
    QList<WorkerProxy*> workers;

    QThread *commiterThread;
    WorkerCommiter *commiter;

//    int maxWorkers;

    int workerCounter;

    QSemaphore *semaphore;

public:
    explicit WorkerPool(QObject *parent = 0);

    int nextRoundRobbin() {if (++workerCounter >= workers.size()) workerCounter = 0; return workerCounter; }

signals:
    void produced(PMessage msg);
    void error(PMessage message, QString err);
    void rollback(PMessage message);

public slots:
    void produce(PMessage msg);
//    void setMaxWorkers(int value);
    int init();
    int initCommiter();
    int append(WorkerProxy *worker);

    WorkerCommiter *getCommiter();

};

#endif // WORKERPOOL_H
