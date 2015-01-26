#ifndef QQUEUECONSUMER_H
#define QQUEUECONSUMER_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include "message.h"

class QQueueConsumer : public QObject
{
    Q_OBJECT
    QQueue<Message> *queue;
    QMutex mutex;

public:
    explicit QQueueConsumer(QObject *parent);
    QQueueConsumer(QQueue<Message> *_queue);

signals:
    void message(Message msg);
public slots:
    void consume();
};

#endif // QQUEUECONSUMER_H
