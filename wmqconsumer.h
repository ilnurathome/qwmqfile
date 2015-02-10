#ifndef WMQCONSUMER_H
#define WMQCONSUMER_H

#include "wmqconnection.h"
#include "bytemessage.h"
#include <QRunnable>
#include <QObject>
#include <QHash>

MQLONG parseMQRFHeader2(char* pszDataPointer, size_t len, QHash<QString, QString> &props);

/**
 * @brief The WMQConsumer class
 *
 * Use case: Get message from queue by WMQConsumer and write it to file by FileProducer

 * Consumer in multithreaded model. Manual create in custom function
 *
 *             +-<<Thread1>>- WMQConsumer 1 - WMQProducer 1
 *             |
 * one queue --+-<<Thread2>>- WMQConsumer 2 - WMQProducer 2
 *             |
 *             +-<<Thread3>>- WMQConsumer 3 - WMQProducer 3
 *
 * Example code
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
        QObject::connect(consumer, SIGNAL(message(PMessage)), processor, SLOT(process(PMessage)), Qt::DirectConnection);
        QObject::connect(processor, SIGNAL(proceed(PMessage)), producer, SLOT(produce(PMessage)), Qt::DirectConnection);

        QObject::connect(producer, SIGNAL(produced(PMessage)), consumer, SLOT(commit(PMessage)), Qt::DirectConnection);
        QObject::connect(producer, SIGNAL(rollback(PMessage)), consumer, SLOT(rollback(PMessage)), Qt::DirectConnection);

        wmqConsumers.append(consumer);
        wmqProducers.append(producer);
        testMsgProcessors.append(processor);

        QThreadPool::globalInstance()->start(consumer);
    }

 */
class WMQConsumer : public QObject, public QRunnable
{
    Q_OBJECT

    iConnectionFactory* connectionFactory;
    QString queueName;
    ImqQueue queue;
    iConnection* connection;

    long msgConsumed;
    long msgCommited;
    long msgRollbacked;

    int nConsumer;

    bool isquit;

    bool transacted;

public:
    explicit WMQConsumer(QObject *parent = 0);
    virtual ~WMQConsumer();

    QString getQueueName() const;

    void run();

    iConnectionFactory *getConnectionFactory() const;

    int getNConsumer() const;

    bool getTransacted() const;

signals:
    void message(PMessage msg);
    void error(QString err);

public slots:
    void commit(PMessage msg);
    void rollback(PMessage msg);

    void setQueueName(const QString &value);
    void setConnectionFactory(iConnectionFactory *value);
    void setNConsumer(int value);
    void setTransacted(bool value);

    int init();
    void quit();
};

#endif // WMQCONSUMER_H
