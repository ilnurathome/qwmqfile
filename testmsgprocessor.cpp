#include "testmsgprocessor.h"
#include <QCoreApplication>

TestMsgProcessor::TestMsgProcessor(QObject *parent) :
    QObject(parent)
{
}

void TestMsgProcessor::process(Message *msg)
{
    if(msg) {
        if(msg->getHeaders().contains("FileName")) {
            if(msg->getHeaders().value("FileName") == "exit") {
                QCoreApplication::exit(0);
                return;
            }
        }

        msg->setHeader("test1", "test1");
        msg->setHeader("test2", "test2");
        msg->setHeader("test3", "test3");
        msg->setHeader("test4", "test4");
        msg->setHeader("test5", "test5");
        msg->setHeader("test6", "test6");
    }

    emit proceed(msg);
}
