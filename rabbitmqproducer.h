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

    uint8_t defaultDeliveryMode;
protected:
    void handleError(int r);

public:
    explicit RabbitMQProducer(QObject *parent = 0);
    ~RabbitMQProducer();

//    static bool initScriptEngine(QScriptEngine &engine);

    QString getRoutingKey() const;
    uint8_t getDefaultDeliveryMode() const;

signals:
    void produced(PMessage msg);
    void got(PMessage msg);
    void error(PMessage message, QString err);
    void rollback(PMessage msg);

public slots:
    void produce(PMessage message);
    void setQueueName(const QString &value);
    void setExchangeName(const QString &value);

    void setDefaultDeliveryMode(uint8_t value);
    void setRoutingKey(const QString &value);

    void setConnectionFactory(RabbitMQConnectionFactory *value);

};

#endif // RABBITMQPRODUCER_H
