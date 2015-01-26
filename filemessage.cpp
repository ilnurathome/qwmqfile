#include <QDebug>
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

QByteArray* FileMessage::getBodyAsByteArray()
{
    qDebug() << "FileMessage::getBody";
    QByteArray* bodyArray = new QByteArray();

    if (f) {
        if(f->open(QIODevice::ReadOnly)) {
            bodyArray->append(f->readAll());
            f->close();
        }
    }
    return bodyArray;
}

QFile *FileMessage::getFile()
{
    return f;
}

