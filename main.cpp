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
#include "fileproducer.h"
#include "file2bytearrayprocess.h"
#include "tfileconsumer.h"
#include "httpproducer.h"

#ifndef _WIN32
#include <csignal>
void cleanExit(int sig)
{
    qDebug() << "Shutdown application CTRL+C.";
    QCoreApplication::exit(0);
}

void registerCleanExit()
{
    signal(SIGINT, &cleanExit);
    signal(SIGTERM, &cleanExit);
}
#endif

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
    WMQProducerThreaded::initScriptEngine(*myEngine);
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
    WMQProducerThreaded producer((iConnectionFactory *)&pool);
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

    QThreadPool::globalInstance()->setMaxThreadCount(24);
    qDebug() << "QThreadPool::globalInstance()->maxThreadCount()=" << QThreadPool::globalInstance()->maxThreadCount();

    WMQConnectionFactory connectionFactory;
    connectionFactory.setQueueManagerName("TEST.QM");
    //    connectionFactory.setConnectionName("devmod01v(1414)");
    connectionFactory.setConnectionName("192.168.56.3(1414)");
    connectionFactory.setChannelName("JAVA.CHANNEL");

    ConnectionPool pool;
    pool.setConnectionFactory(&connectionFactory);
    pool.setMaxConnections(24);


    QList<WMQConsumer*> wmqConsumers;
    QList<TestMsgProcessor*> testMsgProcessors;
    QList<WMQProducer*> wmqProducers;

    for(int i=0; i<8; i++) {
        TestMsgProcessor *processor = new TestMsgProcessor();

        WMQProducer *producer = new WMQProducer((iConnectionFactory *)&pool);
        producer->setQueueName("Q");

        WMQConsumer *consumer = new WMQConsumer();
        consumer->setQueueName("Q1");
        consumer->setConnectionFactory((iConnectionFactory *)&pool);
        consumer->setNConsumer(i);
        consumer->setTransacted(true);
        consumer->init();
        consumer->setAutoDelete(false);

        // quit signal
        QObject::connect(&a, SIGNAL(aboutToQuit()), consumer, SLOT(quit()), Qt::DirectConnection);

        // message signals
        QObject::connect(consumer, SIGNAL(message(Message*)), processor, SLOT(process(Message*)), Qt::DirectConnection);
        QObject::connect(processor, SIGNAL(proceed(Message*)), producer, SLOT(produce(Message*)), Qt::DirectConnection);

        QObject::connect(producer, SIGNAL(produced(Message*)), consumer, SLOT(commit(Message*)), Qt::DirectConnection);
        QObject::connect(producer, SIGNAL(rollback(Message*)), consumer, SLOT(rollback(Message*)), Qt::DirectConnection);

        wmqConsumers.append(consumer);
        wmqProducers.append(producer);
        testMsgProcessors.append(processor);

        QThreadPool::globalInstance()->start(consumer);
    }

    FileConsumer fileConsumer;
    fileConsumer.setBatchSize(10000);
    fileConsumer.setPath("/tmp/filewmq/swifts");
    fileConsumer.setArchPath("/tmp/filewmq/arch");
    //    consumer.setPath("c:/temp/filewmq/swifts");
    //    consumer.setArchPath("c:/temp/filewmq/arch");
    fileConsumer.init();

    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &fileConsumer, SLOT(consume()));
    timer.start(500);

    qDebug() << "Create producer 1";
    WMQProducerThreaded wfproducer((iConnectionFactory *)&pool);
    wfproducer.setQueueName("Q1");
    wfproducer.setMaxWorkers(8);
    wfproducer.init();

    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &fileConsumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&fileConsumer, SIGNAL(message(Message*)), &wfproducer, SLOT(produce(Message*)));

    QObject::connect(wfproducer.getCommiter(), SIGNAL(commited(Message*)), fileConsumer.getCommiter(), SLOT(commit(Message*)), Qt::QueuedConnection);
    QObject::connect(wfproducer.getCommiter(), SIGNAL(rollbacked(Message*)), fileConsumer.getCommiter(), SLOT(rollback(Message*)), Qt::QueuedConnection);



    FileProducer fileProducer;
    fileProducer.setPath("/tmp/filewmq/inbox");

    WMQConsumer consumer2file;
    consumer2file.setQueueName("Q2");
    consumer2file.setConnectionFactory((iConnectionFactory *)&pool);
    consumer2file.setNConsumer(100);
    consumer2file.setTransacted(false);
    consumer2file.init();
    consumer2file.setAutoDelete(false);

    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &consumer2file, SLOT(quit()), Qt::DirectConnection);

    // message signals

    QObject::connect(&consumer2file, SIGNAL(message(Message*)), &fileProducer, SLOT(produce(Message*)), Qt::DirectConnection);

    QObject::connect(&fileProducer, SIGNAL(produced(Message*)), &consumer2file, SLOT(commit(Message*)), Qt::DirectConnection);
    QObject::connect(&fileProducer, SIGNAL(rollback(Message*)), &consumer2file, SLOT(rollback(Message*)), Qt::DirectConnection);

    QThreadPool::globalInstance()->start(&consumer2file);

#ifndef _WIN23
    registerCleanExit();
#endif
    int ret = a.exec();

    while(!QThreadPool::globalInstance()->waitForDone(5000))
    {
        qDebug() << "Wait thread pool";
    }

    qDebug() << "Cleanup on exit";

    while(!wmqConsumers.empty()) {
        WMQConsumer *o = wmqConsumers.takeLast();
        if (o)
            delete o;
    }

    while(!testMsgProcessors.empty()) {
        TestMsgProcessor *o = testMsgProcessors.takeLast();
        if (o)
            delete o;
    }

    while(!wmqProducers.empty()) {
        WMQProducer *o = wmqProducers.takeLast();
        if (o)
            delete o;
    }

    return ret;
}


int test_fileconv_tmpl(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TFileConsumer fileConsumer;
    fileConsumer.setBatchSize(10000);
    fileConsumer.setPath("/tmp/filewmq/swifts");
    fileConsumer.setArchPath("/tmp/filewmq/arch");
    //    consumer.setPath("c:/temp/filewmq/swifts");
    //    consumer.setArchPath("c:/temp/filewmq/arch");
    fileConsumer.init();

    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &fileConsumer, SLOT(consume()));
    timer.start(500);

    qDebug() << "Create processor 1";

    TFile2ByteArrayProcess convproc;


    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &fileConsumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&fileConsumer, SIGNAL(message(TMessage<QFile>*)), &convproc, SLOT(process(TMessage<QFile>*)));

    QObject::connect(&convproc, SIGNAL(commited(TMessage<QFile>*)), fileConsumer.getCommiter(), SLOT(commit(TMessage<QFile>*)), Qt::QueuedConnection);
    QObject::connect(&convproc, SIGNAL(rollbacked(TMessage<QFile>*)), fileConsumer.getCommiter(), SLOT(rollback(TMessage<QFile>*)), Qt::QueuedConnection);

#ifndef _WIN23
    registerCleanExit();
#endif
    int ret = a.exec();

    return ret;
}

int test_fileconv(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FileConsumer fileConsumer;
    fileConsumer.setBatchSize(10000);
    fileConsumer.setPath("/tmp/filewmq/swifts");
    fileConsumer.setArchPath("/tmp/filewmq/arch");
    //    consumer.setPath("c:/temp/filewmq/swifts");
    //    consumer.setArchPath("c:/temp/filewmq/arch");
    fileConsumer.init();

    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &fileConsumer, SLOT(consume()));
    timer.start(500);

    HTTPProducer httpproducer;

    qDebug() << "Create processor 1";

    File2ByteArrayProcess convproc;


    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &fileConsumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&fileConsumer, SIGNAL(message(Message*)), &convproc, SLOT(process(Message*)));

    QObject::connect(&convproc, SIGNAL(proceed(Message*)), &httpproducer, SLOT(produce(Message*)));

    QObject::connect(&convproc, SIGNAL(commited(Message*)), fileConsumer.getCommiter(), SLOT(commit(Message*)), Qt::QueuedConnection);
    QObject::connect(&convproc, SIGNAL(rollbacked(Message*)), fileConsumer.getCommiter(), SLOT(rollback(Message*)), Qt::QueuedConnection);

#ifndef _WIN23
    registerCleanExit();
#endif
    int ret = a.exec();

    return ret;
}

int test_filehttpfile(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FileConsumer fileConsumer;
    fileConsumer.setBatchSize(10000);
    fileConsumer.setPath("/tmp/filewmq/swifts");
    fileConsumer.setArchPath("/tmp/filewmq/arch");
    //    consumer.setPath("c:/temp/filewmq/swifts");
    //    consumer.setArchPath("c:/temp/filewmq/arch");
    fileConsumer.init();

    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &fileConsumer, SLOT(consume()));
    timer.start(500);

    HTTPProducer httpproducer;

    qDebug() << "Create processor 1";

    File2ByteArrayProcess convproc;

    FileProducer fileProducer;
    fileProducer.setPath("/tmp/filewmq/inbox");



    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &fileConsumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&fileConsumer, SIGNAL(message(QSharedPointer<Message>)), &convproc, SLOT(process(QSharedPointer<Message>)));

    QObject::connect(&convproc, SIGNAL(proceed(QSharedPointer<Message>)), &httpproducer, SLOT(produce(QSharedPointer<Message>)));

    QObject::connect(&convproc, SIGNAL(commited(QSharedPointer<Message>)), fileConsumer.getCommiter(), SLOT(commit(QSharedPointer<Message>)), Qt::QueuedConnection);
    QObject::connect(&convproc, SIGNAL(rollbacked(QSharedPointer<Message>)), fileConsumer.getCommiter(), SLOT(rollback(QSharedPointer<Message>)), Qt::QueuedConnection);


    QObject::connect(&httpproducer, SIGNAL(produced(QSharedPointer<Message>)), &fileProducer, SLOT(produce(QSharedPointer<Message>)), Qt::DirectConnection);
    //    QObject::connect(&httpproducer, SIGNAL(commited(Message)), &convproc, SLOT(commit(Message)), Qt::DirectConnection);

    //    QObject::connect(&fileProducer, SIGNAL(produced(Message*)), &consumer2file, SLOT(commit(Message*)), Qt::DirectConnection);
    //    QObject::connect(&fileProducer, SIGNAL(rollback(Message*)), &consumer2file, SLOT(rollback(Message*)), Qt::DirectConnection);

#ifndef _WIN23
    registerCleanExit();
#endif
    int ret = a.exec();

    return ret;
}

int main(int argc, char *argv[])
{
    qRegisterMetaType<Message>("Message");
    //    return test_wmq2wmq(argc, argv);
    //        return test_fileconv(argc, argv);
    return test_filehttpfile(argc, argv);
}
