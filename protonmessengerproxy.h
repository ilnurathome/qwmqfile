#ifndef PROTONMESSENGERPROXY_H
#define PROTONMESSENGERPROXY_H

#include <proton/message.h>
#include <proton/messenger.h>

#include <QRunnable>
#include <QScriptEngine>
#include <QObject>
#include "message.h"

class ProtonMessengerProxy : public QObject, public QRunnable
{
    Q_OBJECT

    pn_messenger_t *messenger;

    QString name;

    bool stop;

public:
    explicit ProtonMessengerProxy(QObject *parent = 0);
    ~ProtonMessengerProxy();

    int init();

    void run();

    int subscribe(QString address);

    QString getName() const;

signals:
    void message(PMessage msg);
    void error(QString err);

public slots:
    void commit(PMessage msg);
    void rollback(PMessage msg);

    void produce(PMessage msg);

    void setName(const QString &value);

    void status();
};

#endif // PROTONMESSENGERPROXY_H
