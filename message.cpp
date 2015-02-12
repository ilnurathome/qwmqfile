#include "message.h"
#include <QDebug>

QByteArray Message::getBodyAsByteArray()
{
    return body.toByteArray();
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


QVariant Message::getBody() const
{
    return body;
}

QVariant &Message::getBodyRef()
{
    return body;
}

void Message::setBody(const QVariant &value)
{
    body = value;
}


QHash<QString, QVariant> &Message::getProperties()
{
    return properties;
}

void Message::setProperty(const QString key, const QVariant &value)
{
    properties.insert(key, value);
}

int Message::removeProperty(const QString key)
{
    return properties.remove(key);
}

Message::Message(QVariant value) : claz("Message")
{
    body = value;
}

Message::Message(Message* msg)
{
    //    qDebug() << __PRETTY_FUNCTION__ << ": " << msg;
    if(msg)
        body = msg->getBody();
    //    qDebug() << __PRETTY_FUNCTION__;
}

Message::~Message()
{
//    qDebug() << __PRETTY_FUNCTION__;
}
