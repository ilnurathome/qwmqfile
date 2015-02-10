#include "testmsgprocessor.h"
#include <QCoreApplication>

TestMsgProcessor::TestMsgProcessor(QObject *parent) :
    QObject(parent)
{
}

void TestMsgProcessor::process(PMessage msg)
{
    if(msg.data()) {
        if(msg.data()->getHeaders().contains("FileName")) {
            if(msg.data()->getHeaders().value("FileName") == "exit") {
                QCoreApplication::exit(0);
                return;
            }
        }

        msg.data()->setHeader("test1", "test1");
        msg.data()->setHeader("test2", "test2");
        msg.data()->setHeader("test3", "test3");
        msg.data()->setHeader("test4", "test4");
        msg.data()->setHeader("test5", "test5");
        msg.data()->setHeader("test6", "test6");
        msg.data()->setHeader(MESSAGE_CORRELATION_ID, "test");
    }

    emit proceed(msg);
}
