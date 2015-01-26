#include <QDebug>
#include "qqueueconsumerproducer.h"

QQueueConsumerProducer::QQueueConsumerProducer(QObject *parent) :
    QObject(parent)
{
}

QQueueConsumerProducer::QQueueConsumerProducer(QQueue<Message> *_queue): queue(_queue)
{

}

void QQueueConsumerProducer::consume()
{
    qDebug() << "Try dequeue message internal queue";
    bool dequeue = true;

    while(dequeue) {
//        qDebug() << "Try lock";
        if (!queue->isEmpty()) {
            Message msg = queue->dequeue();
            qDebug() << "Dequeued message internal queue";
            emit message(msg);
        } else {
            dequeue = false;
        }
        mutex.unlock();
    }
}

void QQueueConsumerProducer::produce(Message msg)
{
    mutex.lock();
    queue->enqueue(msg);
    mutex.unlock();
}
