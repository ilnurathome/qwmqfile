#include "rabbitmqconsumer.h"


QString RabbitMQConsumer::getQueueName() const
{
    return queueName;
}

void RabbitMQConsumer::run()
{
//    qDebug() << __PRETTY_FUNCTION__;
    if (!connection)
        connection = connectionFactory->getConnection();

    if (!connection) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't get connection";
        emit error("can't get connection");
        return;
    }

//    qDebug() << __PRETTY_FUNCTION__ << ": connection got";

    isquit = false;

    amqp_basic_consume(connection->getConn(), 1, amqp_qbytearray_bytes(queueName.toUtf8()), amqp_empty_bytes, 0, 0, 0, amqp_empty_table);
    amqp_rpc_reply_t rpcreply = amqp_get_rpc_reply(connection->getConn());
    if(rpcreply.reply_type != AMQP_RESPONSE_NORMAL) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't get rpc reply: " << amqp_rpc_reply_string(rpcreply);
        emit error("can't get rpc reply");
        return;
    }

    while (!isquit) {
        amqp_rpc_reply_t res;
        amqp_envelope_t envelope;

        amqp_maybe_release_buffers(connection->getConn());

        // FIXME!
        timeval tval;
        tval.tv_sec = 5;
        tval.tv_usec = 0;

        res = amqp_consume_message(connection->getConn(), &envelope, &tval, 0);

        if (AMQP_RESPONSE_NORMAL != res.reply_type) {
            if (AMQP_STATUS_OK != res.library_error && AMQP_STATUS_TIMEOUT != res.library_error) {
                qCritical() << __PRETTY_FUNCTION__ << ": consume message error: " << amqp_rpc_reply_string(res);
                emit error("consume message error");
            }
        } else {
            Message *newmsg = amqp_message_conv(envelope.message);
            consumed_delivery_tags.insert(newmsg, envelope.delivery_tag);
            emit message(PMessage(newmsg));
            msgConsumed++;
        }
        amqp_destroy_envelope(&envelope);
    }
}

void RabbitMQConsumer::setQueueName(const QString &value)
{
    queueName = value;
}

RabbitMQConnectionFactory *RabbitMQConsumer::getConnectionFactory() const
{
    return connectionFactory;
}

void RabbitMQConsumer::setConnectionFactory(RabbitMQConnectionFactory *value)
{
    connectionFactory = value;
}

int RabbitMQConsumer::getNConsumer() const
{
    return nConsumer;
}

void RabbitMQConsumer::setNConsumer(int value)
{
    nConsumer = value;
}

bool RabbitMQConsumer::getTransacted() const
{
    return transacted;
}

void RabbitMQConsumer::commit(PMessage msg)
{
//    qDebug() << __PRETTY_FUNCTION__;
    if (!connection)
        connection = connectionFactory->getConnection();

    if (!connection) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't get connection";
        emit error("can't get connection");
        return;
    }

//    qDebug() << __PRETTY_FUNCTION__ << ": connection got";

    if (consumed_delivery_tags.contains(msg.data())) {
        int r = amqp_basic_ack(connection->getConn(), connection->getChannel(), consumed_delivery_tags.value(msg.data()), 0);
        if (r != AMQP_STATUS_OK) {
            qCritical() << __PRETTY_FUNCTION__ << ": AMQP_BASIC_NACK_METHOD ended with: " << amqp_error_string2(r);
        }
        consumed_delivery_tags.remove(msg.data());
    }
}

void RabbitMQConsumer::rollback(PMessage msg)
{
//    qDebug() << __PRETTY_FUNCTION__;
    if (!connection)
        connection = connectionFactory->getConnection();

    if (!connection) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't get connection";
        emit error("can't get connection");
        return;
    }

//    qDebug() << __PRETTY_FUNCTION__ << ": connection got";

    if (consumed_delivery_tags.contains(msg.data())) {
        //    amqp_basic_nack(connection->getConn(), connection->getChannel(), envelope.delivery_tag, 0, 1);
        amqp_basic_nack_t req;
        req.delivery_tag = consumed_delivery_tags.value(msg.data());
        req.multiple = false;
        req.requeue = true;
        int r = amqp_send_method(connection->getConn(), connection->getChannel(), AMQP_BASIC_NACK_METHOD, &req);
        if (r != AMQP_STATUS_OK) {
            qCritical() << __PRETTY_FUNCTION__ << ": AMQP_BASIC_NACK_METHOD ended with: " << amqp_error_string2(r);
        }
        consumed_delivery_tags.remove(msg.data());
    }
}

void RabbitMQConsumer::setTransacted(bool value)
{
    transacted = value;
}

int RabbitMQConsumer::init()
{

}

void RabbitMQConsumer::quit()
{

}

RabbitMQConsumer::RabbitMQConsumer(QObject *parent) :
    QObject(parent), connectionFactory(NULL), connection(NULL), msgConsumed(0), msgCommited(0), msgRollbacked(0), transacted(true)
{
//    qDebug() << __PRETTY_FUNCTION__;
}

RabbitMQConsumer::~RabbitMQConsumer()
{
//    qDebug() << __PRETTY_FUNCTION__;
}


Message *amqp_message_conv(amqp_message_t message)
{
    Message *newmsg = new Message(QByteArray());

    int i;
    for(i=0; i<message.properties.headers.num_entries; i++) {
        newmsg->setHeader(
                    QString(QByteArray((const char*)message.properties.headers.entries[i].key.bytes, (int)message.properties.headers.entries[i].key.len)),
                    QString(QByteArray((const char*)message.properties.headers.entries[i].value.value.bytes.bytes, (int)message.properties.headers.entries[i].value.value.bytes.len)));
    }

    newmsg->setBody(QVariant(QByteArray((const char*)message.body.bytes, (int)message.body.len)));

    return newmsg;
}
