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
    return pn_bytes(ba.size(), ba.constData());
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
