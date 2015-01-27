#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QHash>

class Message
{
protected:
    QHash <QString, QString> headers;
    QObject *msgbody;
    QString emiter;
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
};

#endif // MESSAGE_H
