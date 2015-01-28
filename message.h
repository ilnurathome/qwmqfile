#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QHash>
#include <QByteArray>

#define MESSAGE_CORRELATION_ID "CorrelationId"

class Message
{
protected:
    QHash <QString, QString> headers;
    QObject *msgbody;
    QString emiter;
    QByteArray messageId;

public:
    explicit Message(Message *msg = 0);
    virtual ~Message();

    QObject* getBody();
    void setBody(QObject* value);

    virtual QByteArray &getBodyAsByteArray()=0;

    int setHeader(QString name, QString value);
    QHash<QString, QString>& getHeaders();

    QString getHeader(QString& key);

    QString getEmiter() const;
    void setEmiter(const QString &value);

    QByteArray getMessageId() const;
    QByteArray &getMessageIdRef();
    void setMessageId(const QByteArray &value);
};

#endif // MESSAGE_H
