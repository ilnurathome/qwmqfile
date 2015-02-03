#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QHash>
#include <QByteArray>
#include <QVariant>

#define MESSAGE_CORRELATION_ID "CorrelationId"

class Message
{
protected:
    QHash <QString, QString> headers;
    QVariant body;
    QString emiter;
    QByteArray messageId;

public:
    explicit Message(QVariant value);
    explicit Message(Message *msg = 0);
    virtual ~Message();

    virtual QByteArray getBodyAsByteArray();

    int setHeader(QString name, QString value);
    QHash<QString, QString>& getHeaders();

    QString getHeader(QString& key);

    QString getEmiter() const;
    void setEmiter(const QString &value);

    QByteArray getMessageId() const;
    QByteArray &getMessageIdRef();
    void setMessageId(const QByteArray &value);

    QVariant getBody() const;
    QVariant &getBodyRef();
    void setBody(const QVariant &value);
};

/**
 * example usage
 *
 * FileConsumer emit TMessage<QFile> to File2ByteArrayProcessor
 * File2ByteArrayProcessor convert TMessage<QFile> to TMessage<QByteArray> and emit TMessage<QByteArray> to WMQProducer
 * WMQProducer send to WMQ and emit commit or rollback TMessage<QByteArray>
 * FileConsumer must move on commit or don't move on rollback
 */
template <class T>
class TMessage
{
protected:
    QHash <QString, QByteArray> headers;
    T body;
    QString emiter;
    QByteArray messageId;

    QByteArray msgbodyAsByteArray;

public:
    explicit TMessage() {}
    virtual ~TMessage() {}

    T& getBody() {return body;}

    int setHeader(QString name, QByteArray value) {headers.insert(name,value); return 0;}
    QHash<QString, QByteArray>& getHeaders() {return headers;}

    QByteArray getHeader(QString& key) {return headers.value(key);}

    QString getEmiter() const {return emiter;}
    void setEmiter(const QString &value) {emiter = value;}

    QByteArray getMessageId() const {return messageId;}
    QByteArray &getMessageIdRef() {return messageId;}
    void setMessageId(const QByteArray &value) {messageId = value;}
};

#endif // MESSAGE_H
