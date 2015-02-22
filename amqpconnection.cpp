#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "amqpconnection.h"

pn_bytes_t conv_qbytearray_pn_bytes_t(QByteArray ba)
{
    //    qDebug() << __PRETTY_FUNCTION__ << ": ba=" << ba << "; ba.constData:" << (void*) ba.constData() << "; " << ba.constData();
    //    pn_bytes_t result;
    //    result.size = ba.size();
    //    result.start = ba.constData();
    return pn_bytes(ba.size(), ba.data());
}


QString AMQPConnection::getAddress() const
{
    return address;
}

void AMQPConnection::setAddress(const QString &value)
{
    address = value;
}

pn_messenger_t *AMQPConnection::getMessenger() const
{
    return messenger;
}

AMQPConnection::AMQPConnection() : address("amqp://0.0.0.0"), messenger(NULL)
{
}

AMQPConnection::~AMQPConnection()
{
}

int AMQPConnection::open()
{
    messenger = pn_messenger(NULL);

    if (!messenger) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't create messenger";
        return -1;
    }

    return pn_messenger_start(messenger);
}

int AMQPConnection::close()
{
    if(messenger) {
        int r = pn_messenger_stop(messenger);
        if (r != 0) {
            if (r == PN_INPROGRESS) {
                for (int i=0; i<10 && !pn_messenger_stopped(messenger); i++) {
                    //FIXME
                };
            }
        }
        pn_messenger_free(messenger);
    }
    return 0;
}

int AMQPConnection::reset()
{
    close();
    open();
    return 0;
}


AMQPConnectionFactory::AMQPConnectionFactory(QObject *parent) :
    QObject(parent)
{

}

AMQPConnection *AMQPConnectionFactory::getConnection()
{
    AMQPConnection *conn = new AMQPConnection();
    if (!conn) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't create AMQPConnection";
        return NULL;
    }

    if (address.size()>0) {
        conn->setAddress(address);
    }

    conn->open();

    return conn;
}

int AMQPConnectionFactory::releaseConnection(iConnection *connection)
{
    if(connection) {
        connection->close();
        delete connection;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////

AMQP010Connection::AMQP010Connection(QUrl &url, bool t, const qpid::types::Variant::Map& options) :
    transactional(t), connection(url.toString().toStdString(), options)
{
}

AMQP010Connection::~AMQP010Connection()
{
    try {
        session.sync();
        session.close();
        connection.close();
    } catch(const std::exception& e) {
        qCritical() << __PRETTY_FUNCTION__ << ": " << e.what();
    }
}

int AMQP010Connection::open()
{
    try {
        connection.open();
        session = transactional ? connection.createTransactionalSession() : connection.createSession();
    } catch (const std::exception& e) {
        qCritical() << __PRETTY_FUNCTION__ << ": " << e.what();
    }
}

int AMQP010Connection::close()
{
    try {
        session.sync();
        session.close();
        connection.close();
    } catch (const std::exception& e) {
        qCritical() << __PRETTY_FUNCTION__ << ": " << e.what();
    }
}

qpid::messaging::Connection AMQP010Connection::getConnection()
{
    return connection;
}

qpid::messaging::Session AMQP010Connection::getSession()
{
    return session;
}

bool AMQP010Connection::isTransactional()
{
    return transactional;
}



bool AMQP010ConnectionFactory::getTransactional() const
{
    return transactional;
}

void AMQP010ConnectionFactory::setTransactional(bool value)
{
    transactional = value;
}

QString AMQP010ConnectionFactory::getUsername() const
{
    return username;
}

void AMQP010ConnectionFactory::setUsername(const QString &value)
{
    username = value;
}

QString AMQP010ConnectionFactory::getPassword() const
{
    return password;
}

void AMQP010ConnectionFactory::setPassword(const QString &value)
{
    password = value;
}

QUrl AMQP010ConnectionFactory::getUrl() const
{
    return url;
}

void AMQP010ConnectionFactory::setUrl(const QUrl &value)
{
    url = value;
}

QString AMQP010ConnectionFactory::getProtocol() const
{
    return protocol;
}

void AMQP010ConnectionFactory::setProtocol(const QString &value)
{
    protocol = value;
}

AMQP010ConnectionFactory::AMQP010ConnectionFactory(QObject *parent) :
    QObject(parent), transactional(true)
{
}

AMQP010Connection *AMQP010ConnectionFactory::getConnection()
{
    try {
        qpid::types::Variant::Map options;
        options.insert(qpid::types::Variant::Map::value_type("username", username.toStdString()));
        options.insert(qpid::types::Variant::Map::value_type("password", password.toStdString()));

        if (protocol.size()>0) options.insert(qpid::types::Variant::Map::value_type("protocol", protocol.toStdString()));

        AMQP010Connection *conn = new AMQP010Connection(url, transactional, options);
        if (!conn) {
            qCritical() << __PRETTY_FUNCTION__ << ": can't create AMQPConnection";
            return NULL;
        }

        conn->open();

        return conn;
    } catch (const std::exception& e) {
        qCritical() << __PRETTY_FUNCTION__ << ": " << e.what();
        return NULL;
    }
}

int AMQP010ConnectionFactory::releaseConnection(iConnection *connection)
{
    if(connection) connection->close();
    delete connection;
}
