#include "rabbitmqproducer.h"


QString RabbitMQProducer::getRoutingKey() const
{
    return routingKeyName;
}

void RabbitMQProducer::setRoutingKey(const QString &value)
{
    routingKeyName = value;
}

uint8_t RabbitMQProducer::getDefaultDeliveryMode() const
{
    return defaultDeliveryMode;
}

void RabbitMQProducer::setDefaultDeliveryMode(uint8_t value)
{
    defaultDeliveryMode = value;
}

void RabbitMQProducer::handleError(int r)
{
    switch (r) {
    case AMQP_STATUS_OK: // operation completed successfully
        break;
    case AMQP_STATUS_NO_MEMORY: //could not allocate memory
        break;

    case AMQP_STATUS_BAD_AMQP_DATA: //invalid AMQP data
        break;

    case AMQP_STATUS_UNKNOWN_CLASS: //unknown AMQP class id
        break;

    case AMQP_STATUS_UNKNOWN_METHOD: //unknown AMQP method id
        break;

    case AMQP_STATUS_HOSTNAME_RESOLUTION_FAILED: //hostname lookup failed
        break;

    case AMQP_STATUS_INCOMPATIBLE_AMQP_VERSION: //incompatible AMQP version
        break;

    case AMQP_STATUS_CONNECTION_CLOSED: //connection closed unexpectedly
        break;

    case AMQP_STATUS_BAD_URL: //could not parse AMQP URL
        break;

    case AMQP_STATUS_SOCKET_ERROR: //a socket error occurred
//        if(connectionFactory) connectionFactory->releaseConnection(connection);
//        connection = NULL;
        if(connection) connection->reset();
        break;

    case AMQP_STATUS_INVALID_PARAMETER: //invalid parameter
        break;

    case AMQP_STATUS_TABLE_TOO_BIG: //table too large for buffer
        break;

    case AMQP_STATUS_WRONG_METHOD: //unexpected method received
        break;

    case AMQP_STATUS_TIMEOUT: //request timed out
        break;

    case AMQP_STATUS_TIMER_FAILURE: //system timer has failed
        break;

    case AMQP_STATUS_HEARTBEAT_TIMEOUT: //heartbeat timeout, connection closed
        break;

    case AMQP_STATUS_UNEXPECTED_STATE: //unexpected protocol state
        break;

//    case AMQP_STATUS_SOCKET_CLOSED: //socket is closed
//        break;

//    case AMQP_STATUS_SOCKET_INUSE: //socket already open
//        break;

    case AMQP_STATUS_TCP_ERROR: //a socket error occurred
        break;
    case AMQP_STATUS_TCP_SOCKETLIB_INIT_ERROR: //socket library initialization failed
        break;

    case AMQP_STATUS_SSL_ERROR: //a SSL error occurred
        break;
    case AMQP_STATUS_SSL_HOSTNAME_VERIFY_FAILED: //SSL hostname verification failed
        break;
    case AMQP_STATUS_SSL_PEER_VERIFY_FAILED: //SSL peer cert verification failed
        break;
    case AMQP_STATUS_SSL_CONNECTION_FAILED: //SSL handshake failed
        break;


    default:
        break;
    }
}

RabbitMQProducer::RabbitMQProducer(QObject *parent) : QObject(parent), connectionFactory(NULL), connection(NULL), defaultDeliveryMode(2)
{
}

RabbitMQProducer::~RabbitMQProducer()
{
    if(connection && connectionFactory) {
        connectionFactory->releaseConnection(connection);
        connection = NULL;
    }
}

void RabbitMQProducer::produce(PMessage message)
{
    if (message.data() == NULL) {
        qWarning() << "Message is NULL";
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
        qCritical() << "Can't get connection";
        message.data()->setHeader("emiter", __PRETTY_FUNCTION__);
        //        emit error(message, "can't get connection");
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
            props.delivery_mode = 2; // persistent delivery mode
        else
            props.delivery_mode = 1; // notpersistent delivery mode
    else
        props.delivery_mode = defaultDeliveryMode;

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

    amqp_bytes_t exchange = (msg->getProperties().contains("EXCHANGE"))? amqp_qbytearray_bytes(msg->getProperties().value("EXCHANGE").toString().toUtf8()) :
                                                                         amqp_qbytearray_bytes(exchangeName.toUtf8());

    amqp_bytes_t routingkey = (msg->getProperties().contains("ROUTINGKEY"))? amqp_qbytearray_bytes(msg->getProperties().value("ROUTINGKEY").toString().toUtf8()) :
                                                                             amqp_qbytearray_bytes(routingKeyName.toUtf8());

    int r = amqp_basic_publish (connection->getConn(),
                                connection->getChannel(),
                                exchange,
                                routingkey,
                                mandatory,
                                immediate,
                                &props,
                                amqp_qbytearray_bytes(message.data()->getBodyAsByteArray()));
    if (r != AMQP_STATUS_OK) {
        handleError(r);
        qCritical() << "Can't publish message: " << amqp_error_string2(r);
        //        emit error(message, QString("Can't publish message"));
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

    // FIXME!
    timeval tval;
    tval.tv_sec = 60;
    tval.tv_usec = 0;

    amqp_frame_t frame;
    r = amqp_simple_wait_frame_noblock(connection->getConn(), &frame, &tval);
    if (r != AMQP_STATUS_OK) {
        handleError(r);
        qCritical() << "Can't publish message: " << amqp_error_string2(r);
        //        emit error(message, QString("Can't publish message"));
        emit rollback(message);
        //        if(semaphore) semaphore->release();
        //        inuse = false;
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << ": method name: " << amqp_method_name(frame.payload.method.id);

    // basic.ack - channel is in confirm mode, message was 'dealt with' by the broker
    // basic.return then basic.ack - the message wasn't delievered, but was dealt with
    // channel.close - probably tried to publish to a non-existant exchange, in any case error!
    // connection.close - something really bad happened
    if (frame.payload.method.id != AMQP_BASIC_ACK_METHOD) {
        qCritical() << __PRETTY_FUNCTION__ << ": server error: " << amqp_server_exception_string(frame.payload.method);

        //FIXME
        if (frame.payload.method.id == AMQP_CHANNEL_CLOSE_METHOD) {
            // open new channel and close old
            connection->resetChannel();
        } else if (frame.payload.method.id == AMQP_CONNECTION_CLOSE_METHOD) {
            connectionFactory->releaseConnection(connection);
            connection = NULL;
        } else if (frame.payload.method.id == AMQP_BASIC_RETURN_METHOD) {
            // Read properties
            amqp_simple_wait_frame(connection->getConn(), &frame);
            qDebug() << __PRETTY_FUNCTION__ << ": method name: " << amqp_method_name(frame.payload.method.id);
            // Check that frame is on channel 1, and that its a properties type
            uint64_t body_size = frame.payload.properties.body_size;

            uint64_t body_read = 0;
            // Read body frame
            while (body_read < body_size)
            {
                amqp_simple_wait_frame(connection->getConn(), &frame);
                qDebug() << __PRETTY_FUNCTION__ << ": method name: " << amqp_method_name(frame.payload.method.id);
                // Check frame is on channel 1, and that is a body frame
                body_read += frame.payload.body_fragment.len;
            }

            // Read basic.ack
            amqp_simple_wait_frame(connection->getConn(), &frame);
            qDebug() << __PRETTY_FUNCTION__ << ": method name: " << amqp_method_name(frame.payload.method.id);
            // Check frame is on channel 1, and that its a basic.ack
        }

        qCritical() << "Can't confirm send message";
        //        emit error(message, QString("Can't publish message"));
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

void RabbitMQProducer::setConnectionFactory(RabbitMQConnectionFactory *value)
{
    connectionFactory = value;
}
