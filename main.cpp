#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QScriptEngine>
#include "message.h"
#include "fileconsumer.h"
#include "wmqproducer.h"
#include "connectionpool.h"
#include "qqueueconsumerproducer.h"

QScriptValue qTimerConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new QTimer();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}


bool qTimerInitScriptEngine(QScriptEngine &engine)
{
        QScriptValue ctor = engine.newFunction(qTimerConstructor);
        QScriptValue metaObject = engine.newQMetaObject(&QTimer::staticMetaObject, ctor);
        engine.globalObject().setProperty("QTimer", metaObject);
        return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<Message>("Message");
    QScriptEngine myEngine;

    QString scriptName = "context.qs";

    QFile scriptFile(scriptName);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        QMessageLogger().critical() << "Context script cannot be opened";
        return -1;
    }

    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    FileConsumer::initScriptEngine(myEngine);
    WMQConnectionFactory::initScriptEngine(myEngine);
    ConnectionPool::initScriptEngine(myEngine);
    WMQProducer::initScriptEngine(myEngine);
    qTimerInitScriptEngine(myEngine);

    myEngine.evaluate(contents, scriptName);

    if(myEngine.hasUncaughtException()) {
        QMessageLogger().critical() << myEngine.uncaughtException().toString();
    }

    return a.exec();
}


int mainold(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<Message>("Message");


    FileConsumer consumer;
    consumer.setPath("c:/temp/filewmq/swifts");
    consumer.setArchPath("c:/temp/filewmq/arch");

    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &consumer, SLOT(consume()));
    timer.start(5000);



    WMQConnectionFactory connectionFactory;

    ConnectionPool pool;
    pool.setConnectionFactory(&connectionFactory);
    pool.setMaxConnections(24);

    qDebug() << "Create producer 1";
    WMQProducer producer((iConnectionFactory *)&pool);
    producer.setQueueName("Q");
    producer.setMaxWorkers(16);
    producer.init();

    QObject::connect(&consumer, SIGNAL(message(Message)), &producer, SLOT(produce(Message)));
    QObject::connect(&producer, SIGNAL(produced(Message)), &consumer, SLOT(commit(Message)));
    QObject::connect(&producer, SIGNAL(rollback(Message)), &consumer, SLOT(rollback(Message)));


    return a.exec();
}

/*

    //    QFileSystemWatcher qFileWatcher;
    //    qFileWatcher.addPath("c:/temp/filewmq/arc");
    //    QObject::connect(&qFileWatcher, SIGNAL(directoryChanged(QString)), &consumer, SLOT(consume(QString)));

    //    QQueueConsumerProducer reqQProd(&req);
    //    QObject::connect(&consumer, SIGNAL(message(Message)), &reqQProd, SLOT(produce(Message)));

//    QQueue<Message> req;
//    QQueue<Message> resp;
    //    QThread thread1;
    //    QThread thread2;

    //    QQueueConsumerProducer reqQCons1(&req);
    //    QQueueConsumerProducer reqQCons2(&req);
    //    reqQCons1.moveToThread(&thread1);
    //    reqQCons2.moveToThread(&thread2);
    //    thread1.start();
    //    thread2.start();

    //    QTimer timerReqQCons1;
    //    QTimer timerReqQCons2;

    //    QObject::connect(&timerReqQCons1, SIGNAL(timeout()), &reqQCons1, SLOT(consume()));
    //    QObject::connect(&timerReqQCons2, SIGNAL(timeout()), &reqQCons2, SLOT(consume()));
    //    timerReqQCons1.start(1000);
    //    timerReqQCons2.start(1000);

    //    producer.moveToThread(&thread1);

    //    qDebug() << "Create producer 2";
    //    WMQProducer producer2((iConnectionFactory *)&pool);
    //    producer2.setQueueName("Q2");
    //    producer2.moveToThread(&thread2);

    //    QObject::connect(&reqQCons1, SIGNAL(message(Message)), &producer, SLOT(produce(Message)));
    //    QObject::connect(&producer, SIGNAL(produced(Message)), &reqQCons1, SLOT(commit(Message)));

    //    QObject::connect(&reqQCons2, SIGNAL(message(Message)), &producer2, SLOT(produce(Message)));
    //    QObject::connect(&producer, SIGNAL(produced(Message)), &reqQCons2, SLOT(commit(Message)));

    //    qDebug() << "Create message";
    //    Message* msg = new Message();

    //    msg->setHeader("mcd.Msd", "jms_text");
    //    msg->setHeader("FileName", "text.txt");

    //    QString hello = (QString)"Hello world";
    //    qDebug() << "Created hello: " << hello;

    //    qDebug() << "Set message body";
    //    msg->setBody((QObject *)&hello);

    //    qDebug() << "Do send";
    //    producer.doSend(msg);

    //    if (msg) delete msg;
 */
