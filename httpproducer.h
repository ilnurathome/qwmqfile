#ifndef HTTPPRODUCER_H
#define HTTPPRODUCER_H

#include <QSharedPointer>
#include <QScriptEngine>
#include <QObject>
#include "message.h"

class HTTPProducer : public QObject
{
    Q_OBJECT
public:
    explicit HTTPProducer(QObject *parent = 0);

    static bool initScriptEngine(QScriptEngine &engine);

signals:
    void produced(QSharedPointer<Message> msg);
    void error(QSharedPointer<Message> message, QString err);
    void commited(QSharedPointer<Message> msg);
    void rollbacked(QSharedPointer<Message> msg);

public slots:
    void produce(QSharedPointer<Message> message);

};

#endif // HTTPPRODUCER_H
