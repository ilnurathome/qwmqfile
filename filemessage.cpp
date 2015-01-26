#include "filemessage.h"

FileMessage::FileMessage(Message *parent) :
    Message(parent)
{
}

void FileMessage::setBody(QFile* value)
{
    f = value;
    msgbody = value;
}

QObject* FileMessage::getBody()
{
    return msgbody;
}

QFile *FileMessage::getFile()
{
    return f;
}

