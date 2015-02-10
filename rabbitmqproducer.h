#ifndef RABBITMQPRODUCER_H
#define RABBITMQPRODUCER_H

#include <QCoreApplication>
#include <QScriptEngine>
#include "iconnection.h"
#include "message.h"
#include "rabbitmqconnection.h"

class RabbitMQProducer : public QObject
{
    Q_OBJECT

    RabbitMQConnectionFactory* connectionFactory;
    RabbitMQConnection* connection;

    QString queueName;
    QString exchangeName;
    QString routingKeyName;

public:
    RabbitMQProducer();
    RabbitMQProducer(RabbitMQConnectionFactory *value);
    ~RabbitMQProducer();

//    static bool initScriptEngine(QScriptEngine &engine);

    QString getRoutingKey() const;
    void setRoutingKey(const QString &value);

signals:
    void produced(PMessage msg);
    void got(PMessage msg);
    void error(PMessage message, QString err);
    void rollback(PMessage msg);

public slots:
    void produce(PMessage message);
    void setQueueName(const QString &value);
    void setExchangeName(const QString &value);
    void setWorkerNumber(int n);

    void setConnectionFactory(RabbitMQConnectionFactory *value);

};

#endif // RABBITMQPRODUCER_H
