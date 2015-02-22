#ifndef AMQPCONNECTION_H
#define AMQPCONNECTION_H

#include <proton/message.h>
#include <proton/messenger.h>

#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Duration.h>
#include <qpid/messaging/Message.h>
#include <qpid/messaging/Receiver.h>
#include <qpid/messaging/Sender.h>
#include <qpid/messaging/Session.h>
#include <qpid/messaging/Address.h>
#include <qpid/messaging/FailoverUpdates.h>
#include <qpid/types/Variant.h>

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

class AMQP010Connection : public iConnection
{
    bool transactional;

    qpid::messaging::Connection connection;
    qpid::messaging::Session session;

public:
    AMQP010Connection(QUrl &url, bool transactional=false, const qpid::types::Variant::Map& options = qpid::types::Variant::Map());
    ~AMQP010Connection();

    virtual int open();
    virtual int close();
    int reset();

//    int begin();
//    int commit();
//    int rollback();

    qpid::messaging::Connection getConnection();
    qpid::messaging::Session getSession();

    bool isTransactional();
};

class AMQP010ConnectionFactory : public QObject, public iConnectionFactory
{
    Q_OBJECT

    bool transactional;

    QUrl url;
    QString username;
    QString password;
    QString protocol;

public:
    explicit AMQP010ConnectionFactory(QObject *parent = 0);
    virtual AMQP010Connection *getConnection();
    virtual int releaseConnection(iConnection* connection);

    //    static bool initScriptEngine(QScriptEngine &engine);

    bool getTransactional() const;
    void setTransactional(bool value);

    QString getUsername() const;
    void setUsername(const QString &value);

    QString getPassword() const;
    void setPassword(const QString &value);

    QUrl getUrl() const;
    void setUrl(const QUrl &value);

    QString getProtocol() const;
    void setProtocol(const QString &value);

public slots:
};

extern pn_bytes_t conv_qbytearray_pn_bytes_t(QByteArray ba);


#endif // AMQPCONNECTION_H
