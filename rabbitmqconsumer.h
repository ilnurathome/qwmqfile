#ifndef RABBITMQCONSUMER_H
#define RABBITMQCONSUMER_H

#include <QObject>
#include <QScriptEngine>
#include "iconnection.h"
#include "message.h"
#include "rabbitmqconnection.h"

class RabbitMQConsumer : public QObject, public QRunnable
{
    Q_OBJECT

    RabbitMQConnectionFactory* connectionFactory;
    RabbitMQConnection* connection;

    QString queueName;

    long msgConsumed;
    long msgCommited;
    long msgRollbacked;

    int nConsumer;

    bool isquit;

    bool transacted;

    QHash<Message*, uint64_t> consumed_delivery_tags;

public:
    explicit RabbitMQConsumer(QObject *parent = 0);
    virtual ~RabbitMQConsumer();

    QString getQueueName() const;

    void run();

    RabbitMQConnectionFactory *getConnectionFactory() const;

    int getNConsumer() const;

    bool getTransacted() const;

signals:
    void message(PMessage msg);
    void error(QString err);

public slots:
    void commit(PMessage msg);
    void rollback(PMessage msg);

    void setQueueName(const QString &value);
    void setConnectionFactory(RabbitMQConnectionFactory *value);
    void setNConsumer(int value);
    void setTransacted(bool value);

    int init();
    void quit();
};

Message *amqp_message_conv(amqp_message_t);

#endif // RABBITMQCONSUMER_H
