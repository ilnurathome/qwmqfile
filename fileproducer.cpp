#include <QDebug>
#include <QFile>
#include <QDir>
#include <QUuid>
#include "fileproducer.h"


QString FileProducer::getPath() const
{
    return path;
}

void FileProducer::setPath(const QString &value)
{
    path = value;
}
FileProducer::FileProducer(QObject *parent) :
    QObject(parent)
{
}

void FileProducer::produce(Message *message)
{
    if(message) {
        QString fullPath(path + "/");

        if(message->getHeaders().contains("FileName")) {
            fullPath.append(message->getHeaders().value("FileName"));
        } else if(message->getMessageId().size() > 0) {
            fullPath.append(message->getMessageId().toHex());
        } else {
            fullPath.append((new QUuid())->toString());
        }


        QDir dir(path);
        if (!dir.exists()) {
            dir.mkpath(".");
            qDebug() << __PRETTY_FUNCTION__<< ":Create new dir : " << dir.absolutePath();
        }

        if (QFile::exists(fullPath)) {
            QFile::remove(fullPath);
        }

        QFile file(fullPath);

        if (!file.open(QIODevice::WriteOnly)) {
            emit error(message, QString("File open error %1").arg(fullPath));
            emit rollback(message);
            return;
        }

        file.write(message->getBodyAsByteArray());

        file.close();

    }
    emit produced(message);
}
