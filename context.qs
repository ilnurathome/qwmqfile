print("context initing");

//var conumerThread = new QThread();
//print("new consumer thread");

var consumer = new FileConsumer();
consumer.setBatchSize(10);

//consumer.setPath("c:/temp/filewmq/swifts");
//consumer.setArchPath("c:/temp/filewmq/arch");
consumer.setPath("/tmp/filewmq/swifts");
consumer.setArchPath("/tmp/filewmq/arch");
consumer.init();
print("consumer created");

function archFunc() {
    //QDateTime::currentDateTime().toString("yyyyMMdd_hhmm")
    var d = new Date();

    var mm = d.getMonth() + 1;
    if (mm<10) mm = '0' + mm;

    var dd = d.getDate();
    if (dd<10) dd = '0' + dd;

    var HH = d.getHours();
    if (HH<10) HH = '0' + HH;

    var MM = d.getMinutes();
    if (MM<10) MM = '0' + MM;

    return d.getFullYear() + mm + dd + '_' + HH + MM;
}

consumer.setArchPathFuncGlobal("archFunc");

print("consumer inited");


var connectionFactory = new WMQConnectionFactory();
connectionFactory.setQueueManagerName("TEST.QM");
connectionFactory.setConnectionName("192.168.56.3(1414)");
connectionFactory.setChannelName("CPROGRAM.CHANNEL");
print("connectionFactory inited");

var pool = new ConnectionPool();
pool.setConnectionFactory(connectionFactory);
pool.setMaxConnections(24);

print("pool inited");

var producer = new WMQProducerThreaded();
producer.setConnectionFactory(pool);
producer.setQueueName("Q");
producer.setMaxWorkers(16);
producer.init();

consumer["message(Message)"].connect(producer.produce);

producer.getCommiter()["commited(Message)"].connect(consumer.getCommiter().commit);
producer.getCommiter()["rollbacked(Message)"].connect(consumer.getCommiter().rollback);

//QObject::connect(producer.getCommiter(), SIGNAL(commited(Message)), consumer.getCommiter(), SLOT(commit(Message)));

var timer = new QTimer();

timer["timeout()"].connect(consumer.consume);
timer.start(500);

//consumer.moveToThread(conumerThread);
//conumerThread.start();

print("context inited");
