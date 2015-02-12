#include "amqpproducer.h"
#include "common.h"

QString AMQPProducer::getAddress() const
{
    return address;
}

void AMQPProducer::produce(PMessage message)
{
    if (message.data() == NULL) {
        qWarning() << __PRETTY_FUNCTION__ << ": Message is NULL";
        //        emit error(message, "can't get connection");
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

    Message *msg = message.data();

    if (!connection)
        connection = connectionFactory->getConnection();

    if (!connection) {
        qCritical() << __PRETTY_FUNCTION__ << ": Can't get connection";
        message.data()->setHeader("emiter", __PRETTY_FUNCTION__);
        //        emit error(message, "can't get connection");
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

//    pn_message_t *pnmsg;
//    pnmsg = pn_message();
//    if (!pnmsg) {
//        qCritical() << __PRETTY_FUNCTION__ << ": can't create proton message";
//        emit rollback(message);
//        return;
//    }
//    auto_free<pn_message_t> pnmsg_remover(pnmsg, &pn_message_free);
    auto_free_cptr<pn_message_t> pnmsg(pn_message(), &pn_message_free);
    if (!pnmsg.data()) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't create proton message";
        emit rollback(message);
        return;
    }

    pn_message_set_address(pnmsg, address.toAscii().constData());

    QByteArray ba(message.data()->getBodyAsByteArray());

//    pn_data_put_binary(pn_message_body(pnmsg), pn_bytes(ba.size(), ba.constData()));
    pn_data_put_string(pn_message_body(pnmsg), pn_bytes(ba.size(), ba.constData()));

    pn_messenger_put(connection->getMessenger(), pnmsg);
    if(pn_messenger_errno(connection->getMessenger()))
    {
        qCritical() << __PRETTY_FUNCTION__ << ":" << pn_error_text(pn_messenger_error(connection->getMessenger()));
        emit rollback(message);
        return;
    }

    pn_messenger_send(connection->getMessenger(), -1);
    if(pn_messenger_errno(connection->getMessenger()))
    {
        qCritical() << __PRETTY_FUNCTION__ << ":" << pn_error_text(pn_messenger_error(connection->getMessenger()));
        emit rollback(message);
        return;
    }

    emit produced(message);
}

void AMQPProducer::setAddress(const QString &value)
{
    address = value;
}

void AMQPProducer::setConnectionFactory(AMQPConnectionFactory *value)
{
    connectionFactory = value;
}

AMQPProducer::AMQPProducer(QObject *parent) :
    QObject(parent), connectionFactory(NULL), connection(NULL)
{
}

AMQPProducer::~AMQPProducer()
{
}
