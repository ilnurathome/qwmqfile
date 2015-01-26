#ifndef TESTMSGPROCESSOR_H
#define TESTMSGPROCESSOR_H

#include "message.h"
#include <QObject>

class TestMsgProcessor : public QObject
{
    Q_OBJECT
public:
    explicit TestMsgProcessor(QObject *parent = 0);

signals:
    void proceed(Message *msg);
    void error(Message *msg, QString err);
public slots:
    void process(Message *msg);
};

#endif // TESTMSGPROCESSOR_H
