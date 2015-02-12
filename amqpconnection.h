#ifndef AMQPCONNECTION_H
#define AMQPCONNECTION_H

#include <proton/message.h>
#include <proton/messenger.h>

#include <QtCore>
#include <QCoreApplication>
#include <QScriptEngine>

#include "iconnection.h"


class AMQPConnection : public iConnection
{

    QString address;
    pn_messenger_t *messenger;

public:
    AMQPConnection();
    ~AMQPConnection();

    virtual int open();
    virtual int close();
    int reset();
//    int openChannel();
//    int closeChannel();
//    int resetChannel();

    //amqp_connection_state_t &getConn();

//    int begin();
//    int commit();
//    int rollback();

    QString getAddress() const;
    void setAddress(const QString &value);
    pn_messenger_t *getMessenger() const;
};

class AMQPConnectionFactory : public QObject, public iConnectionFactory
{
    Q_OBJECT

    QString address;
public:
    explicit AMQPConnectionFactory(QObject *parent = 0);
    virtual AMQPConnection *getConnection();
    virtual int releaseConnection(iConnection* connection);

    //    static bool initScriptEngine(QScriptEngine &engine);

public slots:
};

extern pn_bytes_t conv_qbytearray_pn_bytes_t(QByteArray ba);


#endif // AMQPCONNECTION_H
