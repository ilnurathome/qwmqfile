#ifndef FILEMESSAGE_H
#define FILEMESSAGE_H

#include <QFile>
#include "message.h"

class FileMessage : public Message
{
    QFile* f;
public:
    explicit FileMessage(Message *parent = 0);

    void setBody(QFile* file);
    virtual QByteArray* getBodyAsByteArray();
    QFile* getFile();
};

#endif // FILEMESSAGE_H
