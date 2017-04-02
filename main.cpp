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
#include "rabbitmqconnection.h"
#include "rabbitmqproducer.h"
#include "rabbitmqconsumer.h"
#include "workerpool.h"
#include "amqpconnection.h"
#include "amqpproducer.h"
#include "indeponentstorage.h"

#ifndef _WIN32
#include <csignal>
void cleanExit(int sig)
{
    //    qDebug() << "Shutdown application CTRL+C.";
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

    IndeponentMemoryStorage<QString> indeponentMemoryStorage;

    FileConsumer consumer;
    consumer.setBatchSize(10000);
    consumer.setPath("/tmp/filewmq/swifts");
    consumer.setArchPath("/tmp/filewmq/arch");
    consumer.setIndeponentStorage((IndeponentStorage<QString>*)&indeponentMemoryStorage);
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

    QList<WMQProducer*> mqProducers;
    QList<WorkerProxy*> workerProxies;

    WorkerPool workPool;
    workPool.initCommiter();

    for (int i=0; i<4; i++) {
        WMQProducer *producer = new WMQProducer();
        producer->setConnectionFactory(&connectionFactory);
        producer->setQueueName("Q");

        WorkerProxy *proxy = new WorkerProxy();
        proxy->setWorker(producer);
        proxy->n = i;
        workerProxies.append(proxy);
        workPool.append(proxy);
        mqProducers.append(producer);
    }

    workPool.init();

    QObject::connect(&consumer, SIGNAL(message(PMessage)), &workPool, SLOT(produce(PMessage)));

    QObject::connect(workPool.getCommiter(), SIGNAL(commited(PMessage)), consumer.getCommiter(), SLOT(commit(PMessage)));
    QObject::connect(workPool.getCommiter(), SIGNAL(rollbacked(PMessage)), consumer.getCommiter(), SLOT(rollback(PMessage)));

    int ret = a.exec();

    while(!workerProxies.empty()) {
        WorkerProxy *o = workerProxies.takeLast();
        if (o)
            delete o;
    }

    while(!mqProducers.empty()) {
        WMQProducer *o = mqProducers.takeLast();
        if (o)
            delete o;
    }

    return ret;
}

int test_wmq2wmq(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QThreadPool::globalInstance()->setMaxThreadCount(24);
    //    qDebug() << "QThreadPool::globalInstance()->maxThreadCount()=" << QThreadPool::globalInstance()->maxThreadCount();

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

        WMQProducer *producer = new WMQProducer();
        producer->setConnectionFactory((iConnectionFactory *)&pool);
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
        QObject::connect(consumer, SIGNAL(message(PMessage)), processor, SLOT(process(PMessage)), Qt::DirectConnection);
        QObject::connect(processor, SIGNAL(proceed(PMessage)), producer, SLOT(produce(PMessage)), Qt::DirectConnection);

        QObject::connect(producer, SIGNAL(produced(PMessage)), consumer, SLOT(commit(PMessage)), Qt::DirectConnection);
        QObject::connect(producer, SIGNAL(rollback(PMessage)), consumer, SLOT(rollback(PMessage)), Qt::DirectConnection);

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

    //    qDebug() << "Create producer 1";
    WMQProducerThreaded wfproducer;
    wfproducer.setConnectionFactory((iConnectionFactory *)&pool);
    wfproducer.setQueueName("Q1");
    wfproducer.setMaxWorkers(8);
    wfproducer.init();

    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &fileConsumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&fileConsumer, SIGNAL(message(PMessage)), &wfproducer, SLOT(produce(PMessage)));

    QObject::connect(wfproducer.getCommiter(), SIGNAL(commited(PMessage)), fileConsumer.getCommiter(), SLOT(commit(PMessage)), Qt::QueuedConnection);
    QObject::connect(wfproducer.getCommiter(), SIGNAL(rollbacked(PMessage)), fileConsumer.getCommiter(), SLOT(rollback(PMessage)), Qt::QueuedConnection);



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

    QObject::connect(&consumer2file, SIGNAL(message(PMessage)), &fileProducer, SLOT(produce(PMessage)), Qt::DirectConnection);

    QObject::connect(&fileProducer, SIGNAL(produced(PMessage)), &consumer2file, SLOT(commit(PMessage)), Qt::DirectConnection);
    QObject::connect(&fileProducer, SIGNAL(rollback(PMessage)), &consumer2file, SLOT(rollback(PMessage)), Qt::DirectConnection);

    QThreadPool::globalInstance()->start(&consumer2file);

#ifndef _WIN23
    registerCleanExit();
#endif
    int ret = a.exec();

    while(!QThreadPool::globalInstance()->waitForDone(5000))
    {
        //        qDebug() << "Wait thread pool";
    }

    //    qDebug() << "Cleanup on exit";

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

    //    qDebug() << "Create processor 1";

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

    //    qDebug() << "Create processor 1";

    File2ByteArrayProcess convproc;


    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &fileConsumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&fileConsumer, SIGNAL(message(PMessage)), &convproc, SLOT(process(PMessage)));

    QObject::connect(&convproc, SIGNAL(proceed(PMessage)), &httpproducer, SLOT(produce(PMessage)));

    QObject::connect(&convproc, SIGNAL(commited(PMessage)), fileConsumer.getCommiter(), SLOT(commit(PMessage)), Qt::QueuedConnection);
    QObject::connect(&convproc, SIGNAL(rollbacked(PMessage)), fileConsumer.getCommiter(), SLOT(rollback(PMessage)), Qt::QueuedConnection);

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

    //    qDebug() << "Create processor 1";

    File2ByteArrayProcess convproc;

    FileProducer fileProducer;
    fileProducer.setPath("/tmp/filewmq/inbox");



    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &fileConsumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&fileConsumer, SIGNAL(message(PMessage)), &convproc, SLOT(process(PMessage)));

    QObject::connect(&convproc, SIGNAL(proceed(PMessage)), &httpproducer, SLOT(produce(PMessage)));

    QObject::connect(&convproc, SIGNAL(commited(PMessage)), fileConsumer.getCommiter(), SLOT(commit(PMessage)), Qt::QueuedConnection);
    QObject::connect(&convproc, SIGNAL(rollbacked(PMessage)), fileConsumer.getCommiter(), SLOT(rollback(PMessage)), Qt::QueuedConnection);


    QObject::connect(&httpproducer, SIGNAL(produced(PMessage)), &fileProducer, SLOT(produce(PMessage)), Qt::DirectConnection);


    WMQConnectionFactory connectionFactory;
    connectionFactory.setQueueManagerName("TEST.QM");
    connectionFactory.setConnectionName("192.168.56.3(1414)");
    connectionFactory.setChannelName("CPROGRAM.CHANNEL");

    ConnectionPool pool;
    pool.setConnectionFactory(&connectionFactory);
    pool.setMaxConnections(24);

    //    qDebug() << "Create producer 1";
    WMQProducerThreaded wmqproducer;
    wmqproducer.setConnectionFactory((iConnectionFactory *)&pool);
    wmqproducer.setQueueName("Q");
    wmqproducer.setMaxWorkers(4);
    wmqproducer.init();

    QObject::connect(&httpproducer, SIGNAL(produced(PMessage)), &wmqproducer, SLOT(produce(PMessage)));

    //    QObject::connect(wmqproducer.getCommiter(), SIGNAL(commited(QSharedPointer<Message>msg)), consumer.getCommiter(), SLOT(commit(Message)));
    //    QObject::connect(wmqproducer.getCommiter(), SIGNAL(rollbacked(QSharedPointer<Message>msg)), consumer.getCommiter(), SLOT(rollback(Message)));



    //    QObject::connect(&httpproducer, SIGNAL(commited(Message)), &convproc, SLOT(commit(Message)), Qt::DirectConnection);

    //    QObject::connect(&fileProducer, SIGNAL(produced(PMessage)), &consumer2file, SLOT(commit(PMessage)), Qt::DirectConnection);
    //    QObject::connect(&fileProducer, SIGNAL(rollback(PMessage)), &consumer2file, SLOT(rollback(PMessage)), Qt::DirectConnection);

#ifndef _WIN23
    registerCleanExit();
#endif
    int ret = a.exec();

    return ret;
}

int test_badfileconv(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //    qDebug() << "Create processor 1";

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
    consumer.setTransacted(true);
    consumer.init();
    consumer.setAutoDelete(false);

    QThreadPool::globalInstance()->start(&consumer);

    File2ByteArrayProcess convproc;

    //    quit signal
    //    QObject::connect(&a, SIGNAL(aboutToQuit()), &consumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&consumer, SIGNAL(message(PMessage)), &convproc, SLOT(process(PMessage)));

    //    QObject::connect(&convproc, SIGNAL(proceed(PMessage)), &httpproducer, SLOT(produce(PMessage)));

#ifndef _WIN23
    registerCleanExit();
#endif
    int ret = a.exec();

    return ret;
}

int test_file2rabbitmq(int argc, char *argv[])
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

    RabbitMQConnectionFactory connectionFactory;
    connectionFactory.setHostname("localhost");
    //        connectionFactory.setHostname("192.168.56.3");
    connectionFactory.setPort(5672);
    connectionFactory.setLogin("client");
    connectionFactory.setPassword("client");

    //    ConnectionPool pool;
    //    pool.setConnectionFactory(&connectionFactory);
    //    pool.setMaxConnections(24);

    //    qDebug() << "Create producer 1";
    QList<RabbitMQProducer*> mqProducers;
    QList<WorkerProxy*> workerProxies;

    WorkerPool workPool;
    workPool.initCommiter();

    for (int i=0; i<1; i++) {
        RabbitMQProducer *producer = new RabbitMQProducer();
        producer->setConnectionFactory(&connectionFactory);
        producer->setExchangeName("client.Q");
        //        producer->setRoutingKey("#");
        producer->setRoutingKey("Q0");
        producer->setDefaultDeliveryMode(1);
        //        producer->setRoutingKey(QString("Q%1").arg(i));

        WorkerProxy *proxy = new WorkerProxy();
        proxy->setWorker(producer);
        proxy->n = i;
        workerProxies.append(proxy);
        workPool.append(proxy);
    }

    workPool.init();

    QObject::connect(&consumer, SIGNAL(message(PMessage)), &workPool, SLOT(produce(PMessage)));

    QObject::connect(workPool.getCommiter(), SIGNAL(commited(PMessage)), consumer.getCommiter(), SLOT(commit(PMessage)));
    QObject::connect(workPool.getCommiter(), SIGNAL(rollbacked(PMessage)), consumer.getCommiter(), SLOT(rollback(PMessage)));


    //    QObject::connect(&producer, SIGNAL(produced(Message)), &consumer, SLOT(commit(Message)));
    //    QObject::connect(&producer, SIGNAL(rollback(Message)), &consumer, SLOT(rollback(Message)));


    return a.exec();
}

int test_rabbitmq2rabbitmq(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QThreadPool::globalInstance()->setMaxThreadCount(24);
    //    qDebug() << "QThreadPool::globalInstance()->maxThreadCount()=" << QThreadPool::globalInstance()->maxThreadCount();

    RabbitMQConnectionFactory connectionFactory;
    connectionFactory.setHostname("localhost");
    connectionFactory.setPort(5672);
    connectionFactory.setLogin("client");
    connectionFactory.setPassword("client");

    RabbitMQConsumer consumer;
    consumer.setQueueName("client.Q2");
    consumer.setConnectionFactory(&connectionFactory);
    consumer.setNConsumer(0);
    consumer.setTransacted(true);
    consumer.setAutoDelete(false);
    consumer.init();

    RabbitMQProducer producer;
    producer.setConnectionFactory(&connectionFactory);
    producer.setExchangeName("client.Q1");
    producer.setRoutingKey("#");

    TestMsgProcessor processor;

    // quit signal
    QObject::connect(&a, SIGNAL(aboutToQuit()), &consumer, SLOT(quit()), Qt::DirectConnection);

    // message signals
    QObject::connect(&consumer, SIGNAL(message(PMessage)), &processor, SLOT(process(PMessage)), Qt::DirectConnection);
    QObject::connect(&processor, SIGNAL(proceed(PMessage)), &producer, SLOT(produce(PMessage)), Qt::DirectConnection);

    QObject::connect(&producer, SIGNAL(produced(PMessage)), &consumer, SLOT(commit(PMessage)), Qt::DirectConnection);
    QObject::connect(&producer, SIGNAL(rollback(PMessage)), &consumer, SLOT(rollback(PMessage)), Qt::DirectConnection);


    QThreadPool::globalInstance()->start(&consumer);


#ifndef _WIN23
    registerCleanExit();
#endif
    int ret = a.exec();
    return ret;
}

int test_file2amqp(int argc, char *argv[])
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

    AMQPConnectionFactory connectionFactory;

    QList<AMQPProducer*> mqProducers;
    QList<WorkerProxy*> workerProxies;

    WorkerPool workPool;
    workPool.initCommiter();

    for (int i=0; i<1; i++) {
        AMQPProducer *producer = new AMQPProducer();
        producer->setConnectionFactory(&connectionFactory);
        //        producer->setAddress("amqp://client:client@localhost//exchange/client.Q/Q0");
        producer->setAddress("amqp://producer:producer@localhost/producer.Q/Q0");

        WorkerProxy *proxy = new WorkerProxy();
        proxy->setWorker(producer);
        proxy->n = i;
        workerProxies.append(proxy);
        workPool.append(proxy);
    }

    workPool.init();

    QObject::connect(&consumer, SIGNAL(message(PMessage)), &workPool, SLOT(produce(PMessage)));

    QObject::connect(workPool.getCommiter(), SIGNAL(commited(PMessage)), consumer.getCommiter(), SLOT(commit(PMessage)));
    QObject::connect(workPool.getCommiter(), SIGNAL(rollbacked(PMessage)), consumer.getCommiter(), SLOT(rollback(PMessage)));

    return a.exec();
}

int test_file2amqp010(int argc, char *argv[])
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

    AMQP010ConnectionFactory connectionFactory;
    connectionFactory.setUrl(QUrl("localhost"));
    connectionFactory.setUsername("producer");
    connectionFactory.setPassword("producer");
    connectionFactory.setTransactional(false);
    //    connectionFactory.setProtocol("amqp1.0");

    QList<AMQP010Producer*> mqProducers;
    QList<WorkerProxy*> workerProxies;

    WorkerPool workPool;
    workPool.initCommiter();

    for (int i=0; i<1; i++) {
        AMQP010Producer *producer = new AMQP010Producer();
        producer->setConnectionFactory(&connectionFactory);
        producer->setAddress("producer.Q");
        producer->setSubject("Q0");

        WorkerProxy *proxy = new WorkerProxy();
        proxy->setWorker(producer);
        proxy->n = i;
        workerProxies.append(proxy);
        workPool.append(proxy);
    }

    workPool.init();

    QObject::connect(&consumer, SIGNAL(message(PMessage)), &workPool, SLOT(produce(PMessage)));

    QObject::connect(workPool.getCommiter(), SIGNAL(commited(PMessage)), consumer.getCommiter(), SLOT(commit(PMessage)));
    QObject::connect(workPool.getCommiter(), SIGNAL(rollbacked(PMessage)), consumer.getCommiter(), SLOT(rollback(PMessage)));

    return a.exec();
}

int main(int argc, char *argv[])
{
    qRegisterMetaType<Message>("Message");
    qRegisterMetaType<PMessage>("PMessage");
    return test_file2wmq(argc, argv);
    //    return test_wmq2wmq(argc, argv);
    //    return test_fileconv(argc, argv);
    //    return test_filehttpfile(argc, argv);
    //     return test_badfileconv(argc, argv);
    //    return test_file2rabbitmq(argc, argv);
    //    return test_rabbitmq2rabbitmq(argc, argv);
    //        return test_file2amqp(argc, argv);
    //    return test_file2amqp010(argc, argv);
}
