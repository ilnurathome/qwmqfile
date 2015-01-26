#ifndef QQUEUECONSUMERPRODUCER_H
#define QQUEUECONSUMERPRODUCER_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include "message.h"

class QQueueConsumerProducer : public QObject
{
    Q_OBJECT

    QQueue<Message> *queue;
    QMutex mutex;

public:
    explicit QQueueConsumerProducer(QObject *parent = 0);
    QQueueConsumerProducer(QQueue<Message> *_queue);

signals:
    void message(Message msg);

public slots:
    void consume();
    void produce(Message msg);
};

#endif // QQUEUECONSUMERPRODUCER_H
