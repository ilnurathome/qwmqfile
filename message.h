#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QHash>
#include <QByteArray>
#include <QVariant>
#include <QSharedPointer>

#define MESSAGE_CORRELATION_ID "CorrelationId"

/**
 * @brief The Message class
 */
class Message
{
protected:
    // FIXME to QHash<QString, QString> or to QHash<QString, QVariant>
    QHash <QString, QString> headers;
    QVariant body;

    // for trace-debug
    QString emiter;

    // Message id
    QByteArray messageId;

    QString claz;

public:
    /**
     * @brief Message constructor from QVariant value
     * @param value
     */
    explicit Message(QVariant value);

    /**
     * @brief Message constructor from another Message
     * @param msg
     */
    explicit Message(Message *msg = 0);

    /**
     * @brief ~Message destructor
     */
    virtual ~Message();

    /**
     * @brief getBodyAsByteArray
     * Get as QByteArray. Not all type of QVariant can be cast to QByteArray
     * @return
     */
    virtual QByteArray getBodyAsByteArray();

    /**
     * @brief setHeader Set header
     * @param name
     * @param value
     * @return
     */
    int setHeader(QString name, QString value);

    /**
     * @brief getHeaders
     * Get QHash reference of headers
     * @return
     */
    QHash<QString, QString>& getHeaders();

    /**
     * @brief getHeader
     * Get header value by key
     * @param key
     * @return
     */
    QString getHeader(QString& key);

    /**
     * @brief getEmiter
     * Trace-debug not for realese
     * @return
     */
    QString getEmiter() const;
    void setEmiter(const QString &value);

    /**
     * @brief getMessageId
     * Get const copy of message id
     * @return
     */
    QByteArray getMessageId() const;

    /**
     * @brief getMessageIdRef
     * Get message if reference
     * @return
     */
    QByteArray &getMessageIdRef();

    /**
     * @brief setMessageId
     * Set message id
     * @param value
     */
    void setMessageId(const QByteArray &value);

    /**
     * @brief getBody
     * Get const body
     * @return
     */
    QVariant getBody() const;

    /**
     * @brief getBodyRef
     * Get body reference
     * @return
     */
    QVariant &getBodyRef();

    /**
     * @brief setBody
     * Set body
     * @param value
     */
    void setBody(const QVariant &value);
};

/**
 * @brief PMessage
 * Type for qRegisterMetaType<PMessage>("PMessage");
 */
typedef QSharedPointer<Message> PMessage;
typedef QScopedPointer<Message> SMessage;

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
