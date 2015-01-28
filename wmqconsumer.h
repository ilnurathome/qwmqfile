#ifndef WMQCONSUMER_H
#define WMQCONSUMER_H

#include "wmqconnection.h"
#include "bytemessage.h"
#include <QRunnable>
#include <QObject>
#include <QHash>

MQLONG parseMQRFHeader2(char* pszDataPointer, size_t len, QHash<QString, QString> &props);


class WMQConsumer : public QObject, public QRunnable
{
    Q_OBJECT

    iConnectionFactory* connectionFactory;
    QString queueName;
    ImqQueue queue;
    iConnection* connection;

    long msgConsumed;
    long msgCommited;
    long msgRollbacked;

    int nConsumer;

    bool isquit;

    bool transacted;

public:
    explicit WMQConsumer(QObject *parent = 0);
    virtual ~WMQConsumer();

    QString getQueueName() const;

    void run();

    iConnectionFactory *getConnectionFactory() const;

    int getNConsumer() const;

    bool getTransacted() const;

signals:
    void message(Message *msg);
    void error(QString err);

public slots:
    void commit(Message *msg);
    void rollback(Message *msg);

    void setQueueName(const QString &value);
    void setConnectionFactory(iConnectionFactory *value);
    void setNConsumer(int value);
    void setTransacted(bool value);

    int init();
    void quit();
};

#endif // WMQCONSUMER_H
