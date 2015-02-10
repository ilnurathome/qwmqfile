#ifndef FILE2BYTEARRAYPROCESS_H
#define FILE2BYTEARRAYPROCESS_H

#include <QByteArray>
#include <QFile>
#include <QSharedPointer>
#include <QObject>
#include "message.h"
#include "filemessage.h"
#include "bytemessage.h"

/**
 * @brief The File2ByteArrayProcess class
 * Read file to QByteArray
 */
class File2ByteArrayProcess : public QObject
{
    Q_OBJECT

    QHash<Message*, Message*> msgs;

    bool requestReply;

public:
    explicit File2ByteArrayProcess(QObject *parent = 0);

signals:
    void proceed(PMessage msg);
    void error(PMessage msg, QString err);

    void commited(PMessage msg);
    void rollbacked(PMessage msg);

public slots:
    void process(PMessage msg);

    void commit(PMessage msg);
    void rollback(PMessage msg);
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
