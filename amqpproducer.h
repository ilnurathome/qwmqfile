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

#endif // AMQPPRODUCER_H
