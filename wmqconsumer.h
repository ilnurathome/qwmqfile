#ifndef WMQCONSUMER_H
#define WMQCONSUMER_H

#include "wmqconnection.h"
#include "bytemessage.h"
#include <QRunnable>
#include <QObject>
#include <QHash>

int parseMQRFHeader2(char* pszDataPointer, QHash<QString, QString> &props);


class WMQConsumer : public QObject, public QRunnable
{
    Q_OBJECT

    iConnectionFactory* connectionFactory;
    QString queueName;
    ImqQueue queue;
    iConnection* connection;

public:
    explicit WMQConsumer(QObject *parent = 0);

    QString getQueueName() const;

    void run();

    iConnectionFactory *getConnectionFactory() const;

signals:
    void message(Message *msg);
    void error(QString err);

public slots:
    void commit(Message *msg);
    void rollback(Message *msg);
    void setQueueName(const QString &value);
    void setConnectionFactory(iConnectionFactory *value);

    void init();
};

#endif // WMQCONSUMER_H