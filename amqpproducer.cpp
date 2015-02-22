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

    qDebug() << __PRETTY_FUNCTION__ << ": messenger mode: " << ((pn_messenger_is_blocking(connection->getMessenger()))? " " : " not ") << "blocking mode";
    qDebug() << __PRETTY_FUNCTION__ << ": messenger mode: " << ((pn_messenger_is_passive(connection->getMessenger()))? " " : " not ") << "passive";

    // FIXME!
    if (pn_messenger_get_outgoing_window(connection->getMessenger()) < 1)
        pn_messenger_set_outgoing_window(connection->getMessenger(),1);


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
    pn_data_put_string(pn_message_body(pnmsg), pn_bytes(ba.size(), ba.data()));

    pn_messenger_put(connection->getMessenger(), pnmsg);
    if(pn_messenger_errno(connection->getMessenger()))
    {
        qCritical() << __PRETTY_FUNCTION__ << ":" << pn_error_text(pn_messenger_error(connection->getMessenger()));
        emit rollback(message);
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << ": outgoing depth:" << pn_messenger_outgoing(connection->getMessenger());
    qDebug() << __PRETTY_FUNCTION__ << ": incoming depth:" << pn_messenger_incoming(connection->getMessenger());

    pn_tracker_t trackermsg = pn_messenger_outgoing_tracker(connection->getMessenger());
    qDebug() << __PRETTY_FUNCTION__ << ": trackmsg:" << trackermsg;

    qDebug() << __PRETTY_FUNCTION__ << ": delivery " << ((pn_messenger_buffered(connection->getMessenger(), trackermsg))? " " : " not ") << "assoc";

    pn_messenger_send(connection->getMessenger(), -1);
    if(pn_messenger_errno(connection->getMessenger()))
    {
        qCritical() << __PRETTY_FUNCTION__ << ":" << pn_error_text(pn_messenger_error(connection->getMessenger()));
        emit rollback(message);
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << ": tracker status:" << pn_messenger_status(connection->getMessenger(), trackermsg);

    // FIXME move to commit imitation function
    switch (pn_messenger_status(connection->getMessenger(), trackermsg)) {
     case PN_STATUS_UNKNOWN: /**< The tracker is unknown. */
        qCritical() << __PRETTY_FUNCTION__ << ": The tracker is unknown";
        emit rollback(message);
        break;
    case PN_STATUS_PENDING: /**< The message is in flight. For outgoing
                              messages, use ::pn_messenger_buffered to
                              see if it has been sent or not. */
        // FIXME add wait code
        qDebug() << __PRETTY_FUNCTION__ << ": delivery " << ((pn_messenger_buffered(connection->getMessenger(), trackermsg))? " " : " not ") << "assoc";
    case PN_STATUS_ACCEPTED: /**< The message was accepted. */
        emit produced(message);
        break;
    case PN_STATUS_REJECTED: /**< The message was rejected. */
        qCritical() << __PRETTY_FUNCTION__ << ": The message was rejected. Rollback.";
        emit rollback(message);
        break;
    case PN_STATUS_RELEASED: /**< The message was released. */
        //FIXME add wait code
    case PN_STATUS_MODIFIED: /**< The message was modified. */
        //FIXME add wait code
    case PN_STATUS_ABORTED: /**< The message was aborted. */
        qCritical() << __PRETTY_FUNCTION__ << ": The message was aborted. Rollback.";
        emit rollback(message);
        break;
    case PN_STATUS_SETTLED: /**< The remote party has settled the message. */
        // FIXME add wait code
        qCritical() << __PRETTY_FUNCTION__ << ": bad status of tracker";
        emit rollback(message);
        break;
    default:
        qCritical() << __PRETTY_FUNCTION__ << ": bad status of tracker";
        emit rollback(message);
    }

    qDebug() << __PRETTY_FUNCTION__ << ": delivery count:" << pn_message_get_delivery_count(pnmsg);

    qDebug() << __PRETTY_FUNCTION__ << ": outgoing depth:" << pn_messenger_outgoing(connection->getMessenger());
    qDebug() << __PRETTY_FUNCTION__ << ": incoming depth:" << pn_messenger_incoming(connection->getMessenger());

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






QString AMQP010Producer::getAddress() const
{
    return address;
}

void AMQP010Producer::produce(PMessage message)
{
    if (message.data() == NULL) {
        qWarning() << __PRETTY_FUNCTION__ << ": Message is NULL";
        //        emit error(message, "can't get connection");
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

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

    try {
        qpid::messaging::Address qpidAddress;
        qpidAddress.setName(address.toStdString());
        qpidAddress.setSubject(subject.toStdString());

                qpid::messaging::Sender sender(connection->getSession().createSender(qpidAddress));
//        qpid::messaging::Sender sender(connection->getSession().createSender(address.toStdString()));
        qpid::messaging::Message msg;

        (message.data()->getHeaders().contains("durable"))?
                    ((message.data()->getHeader("durable") == "true")? msg.setDurable(true) : msg.setDurable(false) ) :
                    msg.setDurable(durability);

//        msg.setSubject(subject.toStdString());

        QHashIterator<QString, QString> headerIterator(message.data()->getHeaders());
        while (headerIterator.hasNext()) {
            headerIterator.next();
            msg.setProperty(headerIterator.key().toStdString(), headerIterator.value().toStdString());
        }

        QByteArray ba = message.data()->getBodyAsByteArray();

//        msg.setContentType();
        msg.setContent(ba.constData(), ba.size());

        sender.send(msg);

        if (connection->isTransactional())
            connection->getSession().commit();
        else
            connection->getSession().sync();

        emit produced(message);
    } catch (const std::exception& error) {
        qCritical() << __PRETTY_FUNCTION__ << ": " << error.what();
        connection->getSession().rollback();
        emit rollback(message);
        return;
    }
}

void AMQP010Producer::setAddress(const QString &value)
{
    address = value;
}

void AMQP010Producer::setConnectionFactory(AMQP010ConnectionFactory *value)
{
    connectionFactory = value;
}


QString AMQP010Producer::getSubject() const
{
    return subject;
}

void AMQP010Producer::setSubject(const QString &value)
{
    subject = value;
}
AMQP010Producer::AMQP010Producer(QObject *parent) :
    QObject(parent), connectionFactory(NULL), connection(NULL), durability(false)
{
}

AMQP010Producer::~AMQP010Producer()
{
}
