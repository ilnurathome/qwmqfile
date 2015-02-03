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
    void produced(PMessage msg);
    void got(PMessage msg);
    void error(PMessage message, QString err);
    void rollback(PMessage msg);

public slots:
    void produce(PMessage message);

    QString getPath() const;
    void setPath(const QString &value);
};

#endif // FILEPRODUCER_H
