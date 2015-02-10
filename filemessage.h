#ifndef FILEMESSAGE_H
#define FILEMESSAGE_H

#include <QFile>
#include <QByteArray>
#include "message.h"

/**
 * @brief The FileMessage class
 * QFile Message
 */
class FileMessage : public Message
{
    QFile* f;
    QByteArray ba;
public:
    explicit FileMessage(Message *parent = 0);
    ~FileMessage();

    void setBody(QFile* file);
    virtual QByteArray getBodyAsByteArray();
    QFile* getFile();
};

#endif // FILEMESSAGE_H
