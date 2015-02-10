#ifndef TESTMSGPROCESSOR_H
#define TESTMSGPROCESSOR_H

#include "message.h"
#include <QObject>

/**
 * @brief The TestMsgProcessor class
 * Example message processor
 */
class TestMsgProcessor : public QObject
{
    Q_OBJECT
public:
    explicit TestMsgProcessor(QObject *parent = 0);

signals:
    void proceed(PMessage msg);
    void error(PMessage msg, QString err);
public slots:
    void process(PMessage msg);
};

#endif // TESTMSGPROCESSOR_H
