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

    bool doSend(Message *msg);


signals:
    void produced(Message *msg);
    void got(Message *msg);
    void error(Message *message, QString err);
    void rollback(Message *msg);

public slots:
    void produce(Message *message);

    QString getPath() const;
    void setPath(const QString &value);
};

#endif // FILEPRODUCER_H
