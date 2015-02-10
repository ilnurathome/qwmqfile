#include "rabbitmqproducer.h"


QString RabbitMQProducer::getRoutingKey() const
{
    return routingKeyName;
}

void RabbitMQProducer::setRoutingKey(const QString &value)
{
    routingKeyName = value;
}
RabbitMQProducer::RabbitMQProducer() : connection(NULL), connectionFactory(NULL)
{
}

RabbitMQProducer::RabbitMQProducer(RabbitMQConnectionFactory *value) : connection(NULL)
{
    connectionFactory = value;
}

RabbitMQProducer::~RabbitMQProducer()
{
    if(connection && connectionFactory)
        connectionFactory->releaseConnection(connection);
}

void RabbitMQProducer::produce(PMessage message)
{
    if (message.data() == NULL) {
        qWarning() << "Message is NULL";
        emit error(message, "can't get connection");
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

    Message *msg = message.data();

    if (!connection)
        connection = connectionFactory->getConnection();

    if (!connection) {
        qCritical() << "Can't get connection";
        message.data()->setHeader("emiter", __PRETTY_FUNCTION__);
        emit error(message, "can't get connection");
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

    amqp_basic_properties_t props;

    ////////////////////////////////////////////
    QHash<QString, QString> &msgHeaders = msg->getHeaders();
    if (msgHeaders.contains("Content-type"))
        props.content_type = amqp_qbytearray_bytes(msgHeaders.value("Content-type").toUtf8());
    else
        props.content_type = amqp_cstring_bytes("text/plain");

    if (msgHeaders.contains("Delivery-mode"))
        if (msgHeaders.value("Delivery-mode") == "true")
            props.delivery_mode = 2;
        else
            props.delivery_mode = 1;
    else
        props.delivery_mode = 2; // persistent delivery mode
//        props.delivery_mode = 1; /* notpersistent delivery mode */

    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;

    if (msgHeaders.contains("Content-encoding")) {
        props.content_encoding = amqp_qbytearray_bytes(msgHeaders.value("Content-encoding").toUtf8());
        props._flags += AMQP_BASIC_CONTENT_ENCODING_FLAG;
    }
    if (msg->getMessageIdRef().size() > 0) {
        amqp_bytes_t msgid;
        msgid.len = msg->getMessageIdRef().size();
        msgid.bytes = (void*)msg->getMessageIdRef().constData();
        props.message_id = msgid;
        props._flags += AMQP_BASIC_MESSAGE_ID_FLAG;
    }

    if (msgHeaders.contains("user_id")) {
        props.user_id = amqp_qbytearray_bytes(msgHeaders.value("user_id").toUtf8());
        props._flags += AMQP_BASIC_USER_ID_FLAG;
    }

    if (msgHeaders.contains("app_id")) {
        props.app_id = amqp_qbytearray_bytes(msgHeaders["app_id"].toUtf8());
        props._flags += AMQP_BASIC_APP_ID_FLAG;
    }

    if (msgHeaders.contains("cluster_id")) {
        props.cluster_id = amqp_qbytearray_bytes(msgHeaders["cluster_id"].toUtf8());
        props._flags += AMQP_BASIC_CLUSTER_ID_FLAG;
    }

    if (msgHeaders.contains("correlation_id")) {
        props.correlation_id = amqp_qbytearray_bytes(msgHeaders["correlation_id"].toUtf8());
        props._flags += AMQP_BASIC_CORRELATION_ID_FLAG;
    }

    if (msgHeaders.contains("priority")) {
        props.priority = (uint8_t) msgHeaders["priority"].toUInt();
        props._flags += AMQP_BASIC_PRIORITY_FLAG;
    }

    if (msgHeaders.contains("timestamp")) {
        props.timestamp = (uint64_t) msgHeaders["timestamp"].toUInt();
        props._flags += AMQP_BASIC_TIMESTAMP_FLAG;
    }

    if (msgHeaders.contains("Expiration")) {
        props.expiration = amqp_qbytearray_bytes(msgHeaders["Expiration"].toUtf8());
        props._flags += AMQP_BASIC_EXPIRATION_FLAG;
    }

    if (msgHeaders.contains("type")) {
        props.type = amqp_qbytearray_bytes(msgHeaders["type"].toUtf8());
        props._flags += AMQP_BASIC_TYPE_FLAG;
    }

    if (msgHeaders.contains("Reply-to")) {
        props.reply_to = amqp_qbytearray_bytes(msgHeaders["Reply-to"].toUtf8());
        props._flags += AMQP_BASIC_REPLY_TO_FLAG;
    }

    props.headers.num_entries = msgHeaders.size();
    amqp_table_entry_t_ entries[msgHeaders.size()];

    msgHeaders.keys();
    msgHeaders.values();
    QList<QByteArray> tmpHeadersKeys;
    QList<QByteArray> tmpHeadersValues;

    int i = 0;
    QHashIterator<QString, QString> headerIterator(msgHeaders);
    while (headerIterator.hasNext()) {
        headerIterator.next();
        tmpHeadersKeys.append(headerIterator.key().toUtf8());
        entries[i].key = amqp_qbytearray_bytes(tmpHeadersKeys.last());

        entries[i].value.kind = AMQP_FIELD_KIND_UTF8;

        tmpHeadersValues.append(headerIterator.value().toUtf8());
        entries[i].value.value.bytes = amqp_qbytearray_bytes(tmpHeadersValues.last());
//        qDebug() << "entries[" << i << "]: key:" << entries[i].key.bytes << " ; value:" << entries[i].value.value.bytes.bytes;
//        qDebug() << "entries[" << i << "]: key:" << (char*)entries[i].key.bytes << " ; value:" << (char*)entries[i].value.value.bytes.bytes;
        i++;
    }

    props.headers.entries = entries;
    props._flags += AMQP_BASIC_HEADERS_FLAG;

//    qDebug() << "Check headers";
//    for(int i=0; i<msgHeaders.size(); i++) {
//        qDebug() << "entries[" << i << "]: key:" << props.headers.entries[i].key.bytes << " ; value:" << props.headers.entries[i].value.value.bytes.bytes;
//    }

//    qDebug() << "Check headers";
//    for(int i=0; i<msgHeaders.size(); i++) {
//        qDebug() << "entries[" << i << "]: key:" << (char*)props.headers.entries[i].key.bytes << " ; value:" << (char*)props.headers.entries[i].value.value.bytes.bytes;
//    }

//    qDebug() << "tmpHeadersKeys: " << tmpHeadersKeys;
    ////////////////////////////////////////////////////////////


    // mandatory indicate to the broker that the message MUST be routed
    // to a queue. If the broker cannot do this it should respond with
    // a basic.reject method.
    short mandatory = 1;
    // immediate indicate to the broker that the message MUST be delivered
    // to a consumer immediately. If the broker cannot do this it should
    // response with a basic.reject method.
    short immediate = 0;

    int r = amqp_basic_publish (connection->getConn(),
                                connection->getChannel(),
                                amqp_qbytearray_bytes(exchangeName.toUtf8()),
                                amqp_qbytearray_bytes(routingKeyName.toUtf8()),
                                mandatory,
                                immediate,
                                &props,
                                amqp_qbytearray_bytes(message.data()->getBodyAsByteArray()));
    if (r != AMQP_STATUS_OK) {
        qCritical() << "Can't publish message: " << amqp_error_string2(r);
        emit error(message, QString("Can't publish message"));
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

    amqp_frame_t frame;
    r = amqp_simple_wait_frame(connection->getConn(), &frame);
    if (r != AMQP_STATUS_OK) {
        qCritical() << "Can't publish message: " << amqp_error_string2(r);
        emit error(message, QString("Can't publish message"));
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

    // basic.ack - channel is in confirm mode, message was 'dealt with' by the broker
    // basic.return then basic.ack - the message wasn't delievered, but was dealt with
    // channel.close - probably tried to publish to a non-existant exchange, in any case error!
    // connection.close - something really bad happened
    if (frame.payload.method.id != AMQP_BASIC_ACK_METHOD) {
        qCritical() << "Can't confirm send message";
        emit error(message, QString("Can't publish message"));
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

    emit produced(message);
}

void RabbitMQProducer::setQueueName(const QString &value)
{
    queueName = value;
}

void RabbitMQProducer::setExchangeName(const QString &value)
{
    exchangeName = value;
}

void RabbitMQProducer::setWorkerNumber(int n)
{

}

void RabbitMQProducer::setConnectionFactory(RabbitMQConnectionFactory *value)
{
    connectionFactory = value;
}
