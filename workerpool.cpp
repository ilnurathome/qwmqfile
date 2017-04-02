#include <QDebug>
#include "workerpool.h"

WorkerPool::WorkerPool(QObject *parent) :
    QObject(parent), commiterThread(NULL), commiter(NULL), workerCounter(0), semaphore(NULL)
{
}

void WorkerPool::produce(PMessage msg)
{
    //    QTime myTimer;
    //    myTimer.start();
        if (semaphore) semaphore->acquire();
    //    qDebug() << __PRETTY_FUNCTION__ << ": semaphore acquire elapsed" << myTimer.elapsed();

        while (true) {
            if (workers.at(nextRoundRobbin())->produce(msg))
                break;
        }

        //    if(semaphore) semaphore->release();
}

//void WorkerPool::setMaxWorkers(int value)
//{
//    if (maxWorkers == 0) maxWorkers = value;
//    else qCritical() << __PRETTY_FUNCTION__ << ": can't change maxWorkers";
//}

int WorkerPool::initCommiter()
{
    commiter = new WorkerCommiter();
    if (!commiter) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't create commiter";
    }

    commiterThread = new QThread();
    if (!commiterThread) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't create commiter thread";
        return -1;
    }

    commiter->moveToThread(commiterThread);

    commiterThread->start();

    return 0;
}

int WorkerPool::init()
{
    semaphore = new QSemaphore(workers.size());
    if (!semaphore) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't create semaphore";
        return -1;
    }

    QListIterator<WorkerProxy*> w(workers);
    while(w.hasNext()) {
        WorkerProxy* worker = w.next();
        if (worker)
            worker->setSemaphore(semaphore);
    }

    return 0;
}

int WorkerPool::append(WorkerProxy *worker)
{
    if (semaphore) {
        qCritical() << __PRETTY_FUNCTION__ << "Can't add worker after init and create semaphore";
        return -1;
    }

    if (!worker) {
        qCritical() << __PRETTY_FUNCTION__ << ": NULL worker";
        return -1;
    }

    QThread* thread = new QThread();
    if (!thread) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't create thread";
        return -1;
    }
    threads.append(thread);

    worker->moveToThread(thread);

    workers.append(worker);

    QObject::connect(worker, SIGNAL(produced(PMessage)), commiter, SLOT(commit(PMessage)), Qt::QueuedConnection);
    QObject::connect(worker, SIGNAL(rollbacked(PMessage)), commiter, SLOT(rollback(PMessage)), Qt::QueuedConnection);

    thread->start();

    return 0;
}

WorkerCommiter *WorkerPool::getCommiter()
{
    return commiter;
}

WorkerProxy::WorkerProxy(QObject *parent) :
    QObject(parent), inuse(false), semaphore(NULL), worker(NULL)
{
}

QSemaphore *WorkerProxy::getSemaphore() const
{
    return semaphore;
}

void WorkerProxy::setSemaphore(QSemaphore *value)
{
    semaphore = value;
}

QObject *WorkerProxy::getWorker() const
{
    return worker;
}

void WorkerProxy::setWorker(QObject *value)
{
    if (value) {
        QObject::connect(this, SIGNAL(got(PMessage)), this, SLOT(proccess(PMessage)), Qt::QueuedConnection);
        QObject::connect(this, SIGNAL(send(PMessage)), value, SLOT(produce(PMessage)), Qt::DirectConnection);
        QObject::connect(value, SIGNAL(produced(PMessage)), this, SLOT(commit(PMessage)), Qt::DirectConnection);
        QObject::connect(value, SIGNAL(rollback(PMessage)), this, SLOT(rollback(PMessage)), Qt::DirectConnection);
    }
    else
        if (worker) {
            QObject::disconnect(this, SIGNAL(got(PMessage)), worker, SLOT(produce(PMessage)));
            QObject::disconnect(worker, SIGNAL(produced(PMessage)), this, SLOT(commit(PMessage)));
            QObject::disconnect(worker, SIGNAL(rollback(PMessage)), this, SLOT(rollback(PMessage)));
        }

    worker = value;
    qDebug() << __PRETTY_FUNCTION__;
}

bool WorkerProxy::produce(PMessage msg)
{
    QReadLocker locker(&lock);
    if (inuse) {
        return false;
    }

    inuse = true;

//    if(semaphore) semaphore->acquire();

    emit got(msg);

    return true;
}

void WorkerProxy::proccess(PMessage msg)
{
    emit send(msg);
}

void WorkerProxy::commit(PMessage msg)
{
    inuse = false;
    if(semaphore) semaphore->release();
    emit produced(msg);
}

void WorkerProxy::rollback(PMessage msg)
{
    inuse = false;
    if(semaphore) semaphore->release();
    emit rollbacked(msg);
}


bool WorkerCommiter::initScriptEngine(QScriptEngine &engine)
{

}
