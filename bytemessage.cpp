#include "bytemessage.h"

ByteMessage::ByteMessage()
{
}

void ByteMessage::setBody(QByteArray &value)
{
    b = value;
}

void ByteMessage::setBody(QByteArray *value)
{
    b = *value;
}

QByteArray *ByteMessage::getBodyAsByteArray()
{
    return &b;
}
