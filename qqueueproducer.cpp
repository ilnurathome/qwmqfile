#include <QWriteLocker>
#include "qqueueproducer.h"


void QQueueProducer::produce(Message msg)
{
    QWriteLocker locker(&lock);
    queue->enqueue(msg);
}

QQueueProducer::QQueueProducer(QObject *parent) :
    QObject(parent)
{
}

QQueueProducer::QQueueProducer(QQueue<Message> *_queue) : queue(_queue)
{
}
