#ifndef RABBITMQCONNECTION_H
#define RABBITMQCONNECTION_H

#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_ssl_socket.h>
#include <amqp_tcp_socket.h>

#include <QtCore>
#include <QCoreApplication>
#include <QScriptEngine>

#include "iconnection.h"

class RabbitMQConnection : public iConnection
{
    QString hostname;
    int port;
    int status;
    amqp_channel_t channel;

    QString login;
    QString password;

    amqp_socket_t *socket;
    amqp_connection_state_t conn;

public:
    RabbitMQConnection();
    ~RabbitMQConnection();

    virtual int open();
    virtual int close();

    amqp_connection_state_t &getConn();

    int begin();
    int commit();
    int rollback();
    QString getHostname() const;
    void setHostname(const QString &value);
    int getPort() const;
    void setPort(int value);
    QString getLogin() const;
    void setLogin(const QString &value);
    QString getPassword() const;
    void setPassword(const QString &value);
    amqp_channel_t getChannel() const;
    void setChannel(amqp_channel_t value);
};

class RabbitMQConnectionFactory : public QObject, public iConnectionFactory
{
    Q_OBJECT

    QString hostname;
    int port;

    QString login;
    QString password;

public:
    RabbitMQConnectionFactory();
    virtual RabbitMQConnection *getConnection();
    virtual int releaseConnection(iConnection* connection);

    //    static bool initScriptEngine(QScriptEngine &engine);

public slots:

    QString getHostname() const;
    void setHostname(const QString &value);

    int getPort() const;
    void setPort(int value);

    QString getLogin() const;
    void setLogin(const QString &value);

    QString getPassword() const;
    void setPassword(const QString &value);
};

extern amqp_bytes_t amqp_qbytearray_bytes(QByteArray ba);
extern const char *amqp_rpc_reply_string(amqp_rpc_reply_t r);
extern const char *amqp_server_exception_string(amqp_rpc_reply_t r);

#endif // RABBITMQCONNECTION_H
