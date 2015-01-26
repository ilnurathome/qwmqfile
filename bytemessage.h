#ifndef BYTEMESSAGE_H
#define BYTEMESSAGE_H

#include <QByteArray>
#include "message.h"

class ByteMessage : public Message
{
    QByteArray b;
public:
    ByteMessage();

    void setBody(QByteArray &value);
    void setBody(QByteArray *value);
   virtual QByteArray *getBodyAsByteArray();
};

#endif // BYTEMESSAGE_H
