#include <QDebug>
#include <QMutexLocker>
#include "qqueueconsumer.h"

QQueueConsumer::QQueueConsumer(QObject *parent) :
    QObject(parent)
{
}

QQueueConsumer::QQueueConsumer(QQueue<Message> *_queue) : queue(_queue)
{
}

void QQueueConsumer::consume()
{
    qDebug() << "Try dequeue message internal queue";
    bool dequeue = true;

    while(dequeue) {
//        qDebug() << "Try lock";
        QMutexLocker locker(&mutex);
        if (!queue->isEmpty()) {
            Message msg = queue->dequeue();
            qDebug() << "Dequeued message internal queue";
            emit message(msg);
        } else {
            dequeue = false;
        }
    }
}
