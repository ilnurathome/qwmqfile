#include <QDebug>
#include "bytemessage.h"

ByteMessage::ByteMessage()
{
    claz = "ByteMessage";
//    qDebug() << __PRETTY_FUNCTION__;
}

ByteMessage::~ByteMessage()
{
//    qDebug() << __PRETTY_FUNCTION__;
    b.clear();
}

void ByteMessage::setBody(QByteArray &value)
{
    b = value;
}

void ByteMessage::setBody(QByteArray *value)
{
    b = *value;
}

QByteArray ByteMessage::getBodyAsByteArray()
{
    return b;
}
