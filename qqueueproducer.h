#ifndef QQUEUEPRODUCER_H
#define QQUEUEPRODUCER_H

#include <QObject>
#include <QQueue>
#include <QReadWriteLock>
#include "message.h"

class QQueueProducer : public QObject
{
    Q_OBJECT

    QQueue<Message> *queue;
    QReadWriteLock lock;
public:
    explicit QQueueProducer(QObject *parent);
    QQueueProducer(QQueue<Message> *_queue);

public slots:
    void produce(Message &msg);
};

#endif // QQUEUEPRODUCER_H
