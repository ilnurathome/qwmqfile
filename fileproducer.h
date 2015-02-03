#ifndef FILEPRODUCER_H
#define FILEPRODUCER_H

#include <QString>
#include <QScriptEngine>

#include <QObject>
#include "message.h"

class FileProducer : public QObject
{
    Q_OBJECT

    QString path;
public:
    explicit FileProducer(QObject *parent = 0);

    static bool initScriptEngine(QScriptEngine &engine);


signals:
    void produced(QSharedPointer<Message> msg);
    void got(QSharedPointer<Message> msg);
    void error(QSharedPointer<Message> message, QString err);
    void rollback(QSharedPointer<Message> msg);

public slots:
    void produce(QSharedPointer<Message> message);

    QString getPath() const;
    void setPath(const QString &value);
};

#endif // FILEPRODUCER_H
