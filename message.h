#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QHash>

class Message
{
protected:
    QHash <QString, QString> headers;
    QObject *msgbody;
public:
    explicit Message(Message *msg = 0);
    ~Message();

    QObject* getBody();
    void setBody(QObject* value);

    int setHeader(QString name, QString value);
    QHash<QString, QString>& getHeaders();

    QString getHeader(QString& key);
};

#endif // MESSAGE_H
