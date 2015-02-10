#ifndef HTTPPRODUCER_H
#define HTTPPRODUCER_H

#include <QSharedPointer>
#include <QScriptEngine>
#include <QObject>
#include "message.h"

/**
 * @brief The HTTPProducer class
 * Draft HTTP post client
 *
 * Use case: Get message from queue by WMQConsumer and make request http by HTTPProducer
 */
class HTTPProducer : public QObject
{
    Q_OBJECT
public:
    explicit HTTPProducer(QObject *parent = 0);

    static bool initScriptEngine(QScriptEngine &engine);

signals:
    void produced(PMessage msg);
    void error(PMessage message, QString err);
    void commited(PMessage msg);
    void rollbacked(PMessage msg);

public slots:
    void produce(PMessage message);

};

#endif // HTTPPRODUCER_H
