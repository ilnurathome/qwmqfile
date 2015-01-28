#include "message.h"
#include <QDebug>

QObject* Message::getBody()
{
    return msgbody;
}

void Message::setBody(QObject* value)
{
    msgbody = value;
}

int Message::setHeader(QString name, QString value)
{
    headers.insert(name, value);
    return 0;
}


QHash<QString, QString>& Message::getHeaders()
{
    return headers;
}

QString Message::getHeader(QString& key)
{
    return headers.value(key);
}


QString Message::getEmiter() const
{
    return emiter;
}

void Message::setEmiter(const QString &value)
{
    emiter = value;
}

QByteArray Message::getMessageId() const
{
    return messageId;
}

QByteArray &Message::getMessageIdRef()
{
    return messageId;
}

void Message::setMessageId(const QByteArray &value)
{
    messageId = value;
}

Message::Message(Message* msg)
{
    if(msg)
        msgbody = msg->getBody();
    //    qDebug() << __PRETTY_FUNCTION__;
}

Message::~Message()
{
//    qDebug() << __PRETTY_FUNCTION__;
}
