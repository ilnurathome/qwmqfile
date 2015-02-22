#ifndef AMQPPRODUCER_H
#define AMQPPRODUCER_H

#include <QObject>
#include <QScriptEngine>
#include "message.h"
#include "amqpconnection.h"

class AMQPProducer : public QObject
{
    Q_OBJECT

    AMQPConnectionFactory* connectionFactory;
    AMQPConnection* connection;

    QString address;
public:
    explicit AMQPProducer(QObject *parent = 0);
    ~AMQPProducer();

    QString getAddress() const;

signals:
    void produced(PMessage msg);
    void got(PMessage msg);
    void error(PMessage message, QString err);
    void rollback(PMessage msg);

public slots:
    void produce(PMessage message);
    void setAddress(const QString &value);

    void setConnectionFactory(AMQPConnectionFactory *value);
};

class AMQP010Producer : public QObject
{
    Q_OBJECT

    AMQP010ConnectionFactory* connectionFactory;
    AMQP010Connection* connection;

    QString address;
    QString subject;

    bool durability;

public:
    explicit AMQP010Producer(QObject *parent = 0);
    ~AMQP010Producer();

    QString getAddress() const;

    QString getSubject() const;
    void setSubject(const QString &value);

signals:
    void produced(PMessage msg);
    void got(PMessage msg);
    void error(PMessage message, QString err);
    void rollback(PMessage msg);

public slots:
    void produce(PMessage message);
    void setAddress(const QString &value);

    void setConnectionFactory(AMQP010ConnectionFactory *value);
};

#endif // AMQPPRODUCER_H
