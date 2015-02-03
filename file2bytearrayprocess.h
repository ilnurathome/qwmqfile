#ifndef FILE2BYTEARRAYPROCESS_H
#define FILE2BYTEARRAYPROCESS_H

#include <QByteArray>
#include <QFile>
#include <QSharedPointer>
#include <QObject>
#include "message.h"
#include "filemessage.h"
#include "bytemessage.h"

class File2ByteArrayProcess : public QObject
{
    Q_OBJECT

    QHash<Message*, Message*> msgs;

    bool requestReply;

public:
    explicit File2ByteArrayProcess(QObject *parent = 0);

signals:
    void proceed(QSharedPointer<Message> msg);
    void error(QSharedPointer<Message> msg, QString err);

    void commited(QSharedPointer<Message> msg);
    void rollbacked(QSharedPointer<Message> msg);

public slots:
    void process(QSharedPointer<Message> msg);

    void commit(QSharedPointer<Message> msg);
    void rollback(QSharedPointer<Message> msg);
};


class TFile2ByteArrayProcess : public QObject
{
    Q_OBJECT

    QHash<TMessage<QByteArray>*, TMessage<QFile>*> msgs;

    bool requestReply;

public:
    explicit TFile2ByteArrayProcess(QObject *parent = 0);

signals:
    void proceed(TMessage<QByteArray> *msg);
    void error(TMessage<QFile> *msg, QString err);

    void commited(TMessage<QFile> *msg);
    void rollbacked(TMessage<QFile> *msg);

public slots:
    void process(TMessage<QFile> *msg);

    void commit(TMessage<QByteArray> *msg);
    void rollback(TMessage<QByteArray> *msg);
};

#endif // FILE2BYTEARRAYPROCESS_H
