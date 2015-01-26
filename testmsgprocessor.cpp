#include "testmsgprocessor.h"

TestMsgProcessor::TestMsgProcessor(QObject *parent) :
    QObject(parent)
{
}

void TestMsgProcessor::process(Message *msg)
{
    msg->setHeader("test1", "test1");
    msg->setHeader("test2", "test2");
    msg->setHeader("test3", "test3");
    msg->setHeader("test4", "test4");
    msg->setHeader("test5", "test5");
    msg->setHeader("test6", "test6");

    emit proceed(msg);
}
