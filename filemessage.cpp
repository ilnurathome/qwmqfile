#include "filemessage.h"

FileMessage::FileMessage(Message *parent) :
    Message(parent)
{
}

void FileMessage::setBody(QFile& file)
{

}

QObject* FileMessage::getBody()
{
    return msgbody;
}

