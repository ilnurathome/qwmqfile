#include <QDebug>
#include "filemessage.h"

FileMessage::FileMessage(Message *parent) :
    Message(parent), f(NULL)
{
}

FileMessage::~FileMessage()
{
    qDebug() << __PRETTY_FUNCTION__<< ":delete FileMessage: " << this;
    if(f) {
        if(f->isOpen()) f->close();
        delete f;
    }
}

void FileMessage::setBody(QFile* value)
{
    f = value;
}

QByteArray FileMessage::getBodyAsByteArray()
{
    //    qDebug() << __PRETTY_FUNCTION__;
    if (f && ba.size() == 0) {
        if(f->open(QIODevice::ReadOnly)) {
            ba.append(f->readAll());
            f->close();
        }
    }
    return ba;
}

QFile *FileMessage::getFile()
{
    return f;
}

