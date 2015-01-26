print("context initing");

var consumer = new FileConsumer();
print("consumer created");

consumer.setPath("c:/temp/filewmq/swifts");
consumer.setArchPath("c:/temp/filewmq/arch");

print("consumer inited");


var connectionFactory = new WMQConnectionFactory();
print("connectionFactory inited");

var pool = new ConnectionPool();
pool.setConnectionFactory(connectionFactory);
pool.setMaxConnections(24);

print("pool inited");

var producer = new WMQProducer();
producer.setConnectionFactory(pool);
producer.setQueueName("Q");
producer.setMaxWorkers(16);
producer.init();

consumer["message(Message)"].connect(producer.produce);
producer["produced(Message)"].connect(consumer.commit);
producer["rollback(Message)"].connect(consumer.rollback);

//QObject::connect(&consumer, SIGNAL(message(Message)), &producer, SLOT(produce(Message)));
//QObject::connect(&producer, SIGNAL(produced(Message)), &consumer, SLOT(commit(Message)));
//QObject::connect(&producer, SIGNAL(rollback(Message)), &consumer, SLOT(rollback(Message)));

var timer = new QTimer;

timer["timeout()"].connect(consumer.consume);
timer.start(5000);

print("context inited");
