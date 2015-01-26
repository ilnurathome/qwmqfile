#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QScriptEngine>
#include "common.h"
#include "message.h"
#include "fileconsumer.h"
#include "wmqproducer.h"
#include "connectionpool.h"
#include "wmqconsumer.h"
#include "testmsgprocessor.h"

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

QScriptValue qThreadConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new QThread();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}


bool qThreadInitScriptEngine(QScriptEngine &engine)
{
    QScriptValue ctor = engine.newFunction(qThreadConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&QThread::staticMetaObject, ctor);
    engine.globalObject().setProperty("QThread", metaObject);
    return true;
}

QScriptEngine* myEngine;


int testQS(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //    qRegisterMetaType<Message>("Message");

    QString scriptName = "context.qs";

    QFile scriptFile(scriptName);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Context script cannot be opened";
        return -1;
    }

    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    myEngine = new QScriptEngine();

    FileConsumer::initScriptEngine(*myEngine);
    FileConsumerCommiter::initScriptEngine(*myEngine);
    WMQConnectionFactory::initScriptEngine(*myEngine);
    ConnectionPool::initScriptEngine(*myEngine);
    WMQProducer::initScriptEngine(*myEngine);
    WMQProducerCommiter::initScriptEngine(*myEngine);
    qTimerInitScriptEngine(*myEngine);
    qThreadInitScriptEngine(*myEngine);

    myEngine->evaluate(contents, scriptName);

    if(myEngine->hasUncaughtException()) {
        qCritical() << myEngine->uncaughtException().toString();
    }

    return a.exec();
}


int test_file2wmq(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FileConsumer consumer;
    consumer.setBatchSize(10000);
    consumer.setPath("/tmp/filewmq/swifts");
    consumer.setArchPath("/tmp/filewmq/arch");
    //    consumer.setPath("c:/temp/filewmq/swifts");
    //    consumer.setArchPath("c:/temp/filewmq/arch");
    consumer.init();

    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &consumer, SLOT(consume()));
    timer.start(500);

    WMQConnectionFactory connectionFactory;
    connectionFactory.setQueueManagerName("TEST.QM");
    connectionFactory.setConnectionName("192.168.56.3(1414)");
    connectionFactory.setChannelName("CPROGRAM.CHANNEL");

    ConnectionPool pool;
    pool.setConnectionFactory(&connectionFactory);
    pool.setMaxConnections(24);

    qDebug() << "Create producer 1";
    WMQProducer producer((iConnectionFactory *)&pool);
    producer.setQueueName("Q");
    producer.setMaxWorkers(1);
    producer.init();

    QObject::connect(&consumer, SIGNAL(message(Message)), &producer, SLOT(produce(Message)));

    QObject::connect(producer.getCommiter(), SIGNAL(commited(Message)), consumer.getCommiter(), SLOT(commit(Message)));
    QObject::connect(producer.getCommiter(), SIGNAL(rollbacked(Message)), consumer.getCommiter(), SLOT(rollback(Message)));


    //    QObject::connect(&producer, SIGNAL(produced(Message)), &consumer, SLOT(commit(Message)));
    //    QObject::connect(&producer, SIGNAL(rollback(Message)), &consumer, SLOT(rollback(Message)));


    return a.exec();
}

int test_wmq2wmq(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    WMQConnectionFactory connectionFactory;
    connectionFactory.setQueueManagerName("TEST.QM");
    //    connectionFactory.setConnectionName("devmod01v(1414)");
    connectionFactory.setConnectionName("192.168.56.3(1414)");
    connectionFactory.setChannelName("JAVA.CHANNEL");

    ConnectionPool pool;
    pool.setConnectionFactory(&connectionFactory);
    pool.setMaxConnections(24);


    WMQConsumer consumer;
    consumer.setQueueName("Q1");
    consumer.setConnectionFactory((iConnectionFactory *)&pool);
    consumer.init();

    //    QTimer timer;
    //    QObject::connect(&timer, SIGNAL(timeout()), &consumer, SLOT(consume()));
    //    timer.start(500);

    qDebug() << "Create producer 1";
    WMQProducer producer((iConnectionFactory *)&pool);
    producer.setQueueName("Q2");
    producer.setMaxWorkers(1);
    producer.init();

    WMQProducer producer2((iConnectionFactory *)&pool);
    producer2.setQueueName("Q");
    producer2.setMaxWorkers(1);
    producer2.init();

    TestMsgProcessor processor;

    QObject::connect(&consumer, SIGNAL(message(Message*)), &processor, SLOT(process(Message*)));
    QObject::connect(&processor, SIGNAL(proceed(Message*)), &producer, SLOT(produce(Message*)));

    QObject::connect(&consumer, SIGNAL(message(Message*)), &producer2, SLOT(produce(Message*)));

    QObject::connect(producer.getCommiter(), SIGNAL(commited(Message*)), &consumer, SLOT(commit(Message*)));
    QObject::connect(producer.getCommiter(), SIGNAL(rollbacked(Message*)), &consumer, SLOT(rollback(Message*)));

    QThreadPool::globalInstance()->start(&consumer);

    return a.exec();
}

int main(int argc, char *argv[])
{
    return test_wmq2wmq(argc, argv);
}
