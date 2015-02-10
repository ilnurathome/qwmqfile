#include <QDebug>
#include "rabbitmqconnection.h"

amqp_bytes_t amqp_qbytearray_bytes(QByteArray ba)
{
//    qDebug() << __PRETTY_FUNCTION__ << ": ba=" << ba << "; ba.constData:" << (void*) ba.constData() << "; " << ba.constData();
    amqp_bytes_t result;
    result.len = ba.size();
    result.bytes = (void *) ba.constData();
    return result;
}

const char *amqp_rpc_reply_string(amqp_rpc_reply_t r)
{
  switch (r.reply_type) {
  case AMQP_RESPONSE_NORMAL:
    return "normal response";

  case AMQP_RESPONSE_NONE:
    return "missing RPC reply type";

  case AMQP_RESPONSE_LIBRARY_EXCEPTION:
    return amqp_error_string2(r.library_error);

  case AMQP_RESPONSE_SERVER_EXCEPTION:
    return amqp_server_exception_string(r);

  default:
    abort();
  }
}

const char *amqp_server_exception_string(amqp_rpc_reply_t r)
{
  int res;
  static char s[512];

  switch (r.reply.id) {
  case AMQP_CONNECTION_CLOSE_METHOD: {
    amqp_connection_close_t *m
      = (amqp_connection_close_t *)r.reply.decoded;
    res = snprintf(s, sizeof(s), "server connection error %d, message: %.*s",
                   m->reply_code,
                   (int)m->reply_text.len,
                   (char *)m->reply_text.bytes);
    break;
  }

  case AMQP_CHANNEL_CLOSE_METHOD: {
    amqp_channel_close_t *m
      = (amqp_channel_close_t *)r.reply.decoded;
    res = snprintf(s, sizeof(s), "server channel error %d, message: %.*s",
                   m->reply_code,
                   (int)m->reply_text.len,
                   (char *)m->reply_text.bytes);
    break;
  }

  default:
    res = snprintf(s, sizeof(s), "unknown server error, method id 0x%08X",
                   r.reply.id);
    break;
  }

  return res >= 0 ? s : NULL;
}

QString RabbitMQConnection::getHostname() const
{
    return hostname;
}

void RabbitMQConnection::setHostname(const QString &value)
{
    hostname = value;
}

int RabbitMQConnection::getPort() const
{
    return port;
}

void RabbitMQConnection::setPort(int value)
{
    port = value;
}

QString RabbitMQConnection::getLogin() const
{
    return login;
}

void RabbitMQConnection::setLogin(const QString &value)
{
    login = value;
}

QString RabbitMQConnection::getPassword() const
{
    return password;
}

void RabbitMQConnection::setPassword(const QString &value)
{
    password = value;
}

amqp_channel_t RabbitMQConnection::getChannel() const
{
    return channel;
}

void RabbitMQConnection::setChannel(amqp_channel_t value)
{
    channel = value;
}
RabbitMQConnection::RabbitMQConnection() : login("guest"), password("guest"), conn(NULL), socket(NULL), channel(1)
{
}

RabbitMQConnection::~RabbitMQConnection()
{
    close();
}

int RabbitMQConnection::open()
{
    conn = amqp_new_connection();
    socket = amqp_tcp_socket_new(conn);
    if (!socket) {
        qCritical() << "Can't create amqp socket";
        return -1;
    }

    status = amqp_socket_open(socket, hostname.toUtf8().constData(), port);
    if (status) {
        qCritical() << "Can't opening TCP socket";
        return -2;
    }

    amqp_rpc_reply_t rpcreply = amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, login.toUtf8().constData(), password.toUtf8().constData());
    if (rpcreply.reply_type != AMQP_RESPONSE_NORMAL) {
        qCritical() << "Can't login due: " << rpcreply.reply_type;
        return -3;
    }

    amqp_channel_open(conn, channel);
    rpcreply = amqp_get_rpc_reply(conn);
    if (rpcreply.reply_type != AMQP_RESPONSE_NORMAL) {
        qCritical() << "Can't open channel: " << rpcreply.reply_type;
        return -4;
    }

    amqp_confirm_select(conn, channel);
    rpcreply = amqp_get_rpc_reply(conn);
    if (rpcreply.reply_type != AMQP_RESPONSE_NORMAL) {
        qCritical() << "Can't open channel: " << rpcreply.reply_type;
        return -5;
    }

    return 0;
}

int RabbitMQConnection::close()
{
    if (conn) {
        amqp_rpc_reply_t rpcreply = amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
        if (rpcreply.reply_type != AMQP_RESPONSE_NORMAL) {
            qCritical() << "Can't closing channel due: " << rpcreply.reply_type;
        }

        rpcreply = amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        if (rpcreply.reply_type != AMQP_RESPONSE_NORMAL) {
            qCritical() << "Can't closing connection due: " << rpcreply.reply_type;
        }

        int r = amqp_destroy_connection(conn);
        if (r != AMQP_STATUS_OK) {
            qCritical() << "Can't ending connection: " << rpcreply.reply_type;
        }
        conn = NULL;
        socket = NULL;
    }
}

amqp_connection_state_t &RabbitMQConnection::getConn()
{
    return conn;
}

int RabbitMQConnection::begin()
{

}

int RabbitMQConnection::commit()
{

}

int RabbitMQConnection::rollback()
{

}


int RabbitMQConnectionFactory::getPort() const
{
    return port;
}

void RabbitMQConnectionFactory::setPort(int value)
{
    port = value;
}

QString RabbitMQConnectionFactory::getLogin() const
{
    return login;
}

void RabbitMQConnectionFactory::setLogin(const QString &value)
{
    login = value;
}

QString RabbitMQConnectionFactory::getPassword() const
{
    return password;
}

void RabbitMQConnectionFactory::setPassword(const QString &value)
{
    password = value;
}

RabbitMQConnectionFactory::RabbitMQConnectionFactory()
{
}

RabbitMQConnection *RabbitMQConnectionFactory::getConnection()
{
    RabbitMQConnection *conn = new RabbitMQConnection();

    if (hostname.size() > 0)
        conn->setHostname(hostname);

    if (port)
        conn->setPort(port);

    if (login.size() > 0)
        conn->setLogin(login);

    if (password.size() > 0)
        conn->setPassword(password);

    conn->open();

    return conn;
}

int RabbitMQConnectionFactory::releaseConnection(iConnection *connection)
{
    if(connection) {
        connection->close();
        delete connection;
    }
    return 0;
}

QString RabbitMQConnectionFactory::getHostname() const
{
    return hostname;
}

void RabbitMQConnectionFactory::setHostname(const QString &value)
{
    hostname = value;
}
