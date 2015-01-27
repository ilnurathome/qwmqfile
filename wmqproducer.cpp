#include <stdlib.h>
#include <QByteArray>
#include <QDebug>
#include <cmqpsc.h>
#include "wmqproducer.h"

QScriptValue wmqProducerThreadedConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new WMQProducerThreaded();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

QScriptValue wmqProducerConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new WMQProducer();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

bool WMQProducerThreaded::initScriptEngine(QScriptEngine &engine)
{
    QScriptValue ctor = engine.newFunction(wmqProducerThreadedConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&WMQProducerThreaded::staticMetaObject, ctor);
    engine.globalObject().setProperty("WMQProducerThreaded", metaObject);
    return true;
}

QString WMQProducerThreaded::getQueueName() const
{
    return queueName;
}

void WMQProducerThreaded::setQueueName(const QString &value)
{
    queueName = value;
}

void WMQProducerThreaded::produce(Message *msg)
{
    while (true) {
        if (workers.at(nextRoundRobbin())->doSend(msg))
            break;
    }
}

void WMQProducerThreaded::workerProduced(Message *msg)
{
    msg->setHeader("emiter", "WMQProducer");
    emit produced(msg);
}

void WMQProducerThreaded::getError(Message *message, QString err)
{
    qCritical() << "WMQProducer got error while proccesing message: " << &message << ", err: " << err;
    emit rollback(message);
}


void WMQProducerThreaded::init()
{
    commiterThread = new QThread();
    commiter->moveToThread(commiterThread);

    for (int i=0; i<maxWorkers; i++) {
        QThread* thread = new QThread();
        WMQProducer* worker = new WMQProducer(connectionFactory);
        worker->setWorkerNumber(i);
        worker->setQueueName(queueName);
        worker->moveToThread(thread);

        threads.append(thread);
        workers.append(worker);

        QObject::connect(worker, SIGNAL(produced(Message*)), commiter, SLOT(commit(Message*)), Qt::QueuedConnection);
        QObject::connect(worker, SIGNAL(rollback(Message*)), commiter, SLOT(rollback(Message*)), Qt::QueuedConnection);

        thread->start();
    }

    commiterThread->start();
}

QObject *WMQProducerThreaded::getCommiter()
{
    return commiter;
}


int WMQProducerThreaded::getMaxWorkers() const
{
    return maxWorkers;
}

void WMQProducerThreaded::setMaxWorkers(int value)
{
    maxWorkers = value;
}

iConnectionFactory *WMQProducerThreaded::getConnectionFactory() const
{
    return connectionFactory;
}

void WMQProducerThreaded::setConnectionFactory(iConnectionFactory *value)
{
    connectionFactory = value;
}
WMQProducerThreaded::WMQProducerThreaded(): workerCounter(0)
{
    commiter = new WMQProducerCommiter();
}

WMQProducerThreaded::WMQProducerThreaded(iConnectionFactory* connectionFactory): workerCounter(0)
{
    this->connectionFactory = connectionFactory;
    maxWorkers = 4;
    commiter = new WMQProducerCommiter();
}

WMQProducerThreaded::~WMQProducerThreaded()
{
    QListIterator<QThread*> t(threads);
    while(t.hasNext()) {
        QThread* thread = t.next();
        if (thread)
            delete thread;
    }
    threads.clear();

    QListIterator<WMQProducer*> w(workers);
    while(w.hasNext()) {
        WMQProducer* worker = w.next();
        if (worker)
            delete worker;
    }
    workers.clear();
}

QByteArray* buildMQRFHeader2(Message *msg)
{
    QByteArray* result = new QByteArray();
    if (!result)
        return NULL;

    MQRFH2 RFHeader = {{MQRFH_STRUC_ID_ARRAY}, MQRFH_VERSION_2,MQRFH_STRUC_LENGTH_FIXED_2,
                       MQENC_NATIVE, MQCCSI_INHERIT, {MQFMT_NONE_ARRAY},
                       MQRFH_NONE, 1208};

    QString pscValueString;
    QString pscrValueString;
    QString mcdValueString;
    QString usrValueString;

    QHashIterator<QString, QString> headerIterator(msg->getHeaders());
    while (headerIterator.hasNext()) {
        headerIterator.next();
        QString headerName = headerIterator.key();
        QString headerValue = headerIterator.value();

        if (headerName.startsWith((QString)MQRFH2_PUBSUB_CMD_FOLDER + ".")) {
            QString headerNameTmp = headerName.right(((QString)MQRFH2_PUBSUB_CMD_FOLDER + ".").size()-1);
            pscValueString.append("<" + headerNameTmp +">" + headerValue + "</" + headerNameTmp + ">");
        } else if (headerName.startsWith((QString)MQRFH2_PUBSUB_RESP_FOLDER + ".")) {
            QString headerNameTmp = headerName.right(((QString)MQRFH2_PUBSUB_RESP_FOLDER + ".").size()-1);
            pscrValueString.append("<" + headerNameTmp +">" + headerValue + "</" + headerNameTmp + ">");
        } else if (headerName.startsWith((QString)MQRFH2_MSG_CONTENT_FOLDER + ".")) {
            QString headerNameTmp = headerName.right(((QString)MQRFH2_MSG_CONTENT_FOLDER + ".").size()-1);
            mcdValueString.append("<" + headerNameTmp +">" + headerValue + "</" + headerNameTmp + ">");
        } else {
            usrValueString.append("<" + headerName +">" + headerValue + "</" + headerName + ">");
        }
    }

    QByteArray pscValueData;
    QByteArray pscrValueData;
    QByteArray mcdValueData;
    QByteArray usrValueData;

    //    qDebug() << "Create folders to RFH";
    if (pscValueString.size()>0) {
        pscValueData.append(MQRFH2_PUBSUB_CMD_FOLDER_B);
        pscValueData.append(pscValueString);
        pscValueData.append(MQRFH2_PUBSUB_CMD_FOLDER_E);
        //        qDebug() << "Folder " << MQRFH2_PUBSUB_CMD_FOLDER << " : " << pscValueData;
    }

    if (pscrValueString.size()>0) {
        pscrValueData.append(MQRFH2_PUBSUB_RESP_FOLDER_B);
        pscrValueData.append(pscrValueString);
        pscrValueData.append(MQRFH2_PUBSUB_RESP_FOLDER_E);
        //        qDebug() << "Folder " << MQRFH2_PUBSUB_RESP_FOLDER << " : " << pscrValueData;
    }

    if (mcdValueString.size()>0) {
        mcdValueData.append(MQRFH2_MSG_CONTENT_FOLDER_B);
        mcdValueData.append(mcdValueString);
        mcdValueData.append(MQRFH2_MSG_CONTENT_FOLDER_E);
        //        qDebug() << "Folder " << MQRFH2_MSG_CONTENT_FOLDER << " : " << mcdValueData;
    }

    if (usrValueString.size()>0) {
        usrValueData.append(MQRFH2_USER_FOLDER_B);
        usrValueData.append(usrValueString);
        usrValueData.append(MQRFH2_USER_FOLDER_E);
    }

    int pscValueDataLength = pscValueData.size();
    int pscrValueDataLength = pscrValueData.size();
    int mcdValueDataLength = mcdValueData.size();
    int usrValueDataLength = usrValueData.size();

    if (pscValueDataLength) {
        RFHeader.StrucLength += sizeof(MQINT32) + pscValueDataLength;
    }

    if (pscrValueDataLength) {
        RFHeader.StrucLength += sizeof(MQINT32) + pscrValueDataLength;
    }

    if (mcdValueDataLength) {
        RFHeader.StrucLength += sizeof(MQINT32) + mcdValueDataLength;
    }

    if (usrValueDataLength) {
        RFHeader.StrucLength += sizeof(MQINT32)+ usrValueDataLength;
    }

    //    qDebug() << "Append RFHeader to RFH";
    result->append((const char*) &RFHeader, sizeof(MQRFH2));


    if (pscValueDataLength) {
        //        qDebug() << "Append psc folder size to RFH2: " << pscValueDataLength;
        result->append((const char*)&pscValueDataLength, sizeof (MQINT32));
        //        qDebug() << "Append psc folder to RFH2: " << pscValueData.constData() << endl << pscValueData.toHex();
        result->append(pscValueData);
    }

    if (pscrValueDataLength) {
        //        qDebug() << "Append pscr folder size to RFH2: " << pscrValueDataLength;
        result->append((const char*)&pscrValueDataLength, sizeof (MQINT32));
        //        qDebug() << "Append pscr folder to RFH2: " << pscrValueData.constData() << endl << pscrValueData.toHex();
        result->append(pscrValueData);
    }

    if (mcdValueDataLength) {
        //        qDebug() << "Append mcd folder size to RFH2: " << mcdValueDataLength;
        result->append((const char*)&mcdValueDataLength, sizeof (MQINT32));
        //        qDebug() << "Append mcd folder to RFH2: " << mcdValueData.constData() << endl << mcdValueData.toHex();
        result->append(mcdValueData);
    }

    if (usrValueDataLength) {
        //        qDebug() << "Append usr folder size to RFH2: " << usrValueDataLength;
        result->append((const char*)&usrValueDataLength, sizeof (MQINT32));
        //        qDebug() << "Append usr folder to RFH2: " << usrValueData.constData() << endl << usrValueData.toHex();
        result->append(usrValueData);
    }

    //    qDebug() << "Created RFH2 size: " << result->size() << endl << result->toHex();
    return result;
}

QString WMQProducer::getQueueName() const
{
    return queueName;
}

void WMQProducer::setQueueName(const QString &value)
{
    queueName = value;
}


iConnectionFactory *WMQProducer::getConnectionFactory() const
{
    return connectionFactory;
}

void WMQProducer::setConnectionFactory(iConnectionFactory *value)
{
    connectionFactory = value;
}

WMQProducer::WMQProducer() : inuse(false), connectionFactory(NULL), connection(NULL)
{
    QObject::connect(this, SIGNAL(got(Message*)), this, SLOT(produce(Message*)), Qt::QueuedConnection);
    qDebug() << __PRETTY_FUNCTION__;
}

WMQProducer::WMQProducer(iConnectionFactory *_connectionFactory) : inuse(false), connectionFactory(_connectionFactory), connection(NULL)
{
    QObject::connect(this, SIGNAL(got(Message*)), this, SLOT(produce(Message*)), Qt::QueuedConnection);
    qDebug() << __PRETTY_FUNCTION__;
}

WMQProducer::~WMQProducer()
{
    if(queue.openStatus())
        queue.close();
    if(connectionFactory)
        connectionFactory->releaseConnection(connection);
    qDebug() << __PRETTY_FUNCTION__;
}

bool WMQProducer::initScriptEngine(QScriptEngine &engine)
{
    QScriptValue ctor = engine.newFunction(wmqProducerConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&WMQProducer::staticMetaObject, ctor);
    engine.globalObject().setProperty("WMQProducer", metaObject);
    return true;
}

void WMQProducer::setWorkerNumber(int n)
{
    workerNumber = n;
}

bool WMQProducer::doSend(Message *msg)
{
    QReadLocker locker(&lock);
    if (inuse) {
        //        qDebug() << "WMQProducerThread " << workerNumber << " in use.";
        return false;
    }

    inuse = true;

    msg->setHeader("emiter", "WMQProducerThread");
    emit got(msg);

    return true;
}

void WMQProducer::produce(Message *message)
{
    if (!connection)
        connection = connectionFactory->getConnection();

    if (!connection) {
        message->setHeader("emiter", "WMQProducerThread");
        emit error(message, "can't get connection");
        emit rollback(message);
        return;
    }
//    qDebug() << "WMQProducerThread::send: " << message.getHeaders().value("FileName") << ", workerNumber:" << workerNumber;

    ImqMessage msg;

    if (!queue.openStatus()) {
        queue.setConnectionReference(((WMQConnection*)connection)->getImqQueueManager());
        //    qDebug() << "WMQProducerThread::send: queueName: " << queueName;

        queue.setName(queueName.toStdString().c_str());
        queue.setOpenOptions(MQOO_OUTPUT /* open queue for output        */
                             + MQOO_FAIL_IF_QUIESCING);  /* but not if MQM stopping      */
        queue.open();

        if (queue.reasonCode()) {
            qCritical() << "ImqQueue::open ended with reason code " << (int)queue.reasonCode();
            qCritical() << "ImqQueue::open ended with reason code " << (int)queue.completionCode();
            //        ErrorMessage errmsg;
            //        errmsg.setErrorCode(0);
            //        errmsg.setErrorString("ImqQueue::open error");
            //        emit error(message, errmsg);
            message->setHeader("emiter", "WMQProducerThread");
//            emit error(message, QString("ImqQueue::open error with reason code %1").arg((int)queue.reasonCode()));
            emit rollback(message);
            return;
        }
    }

    QByteArray* bodyArray = new QByteArray();

    if (bodyArray != NULL) {
        QByteArray *rfh2Header = buildMQRFHeader2(message);

        if (rfh2Header) {
            //            qDebug() << "Append rfh2 header to bodyArray ";// << rfh2Header->toHex();
            bodyArray->append(*rfh2Header);
            msg.setFormat(MQFMT_RF_HEADER_2);
        } else {
            msg.setFormat(MQFMT_STRING);
        }

        //        qDebug() << "bodyArray size: " << bodyArray->size();

        bodyArray->append(message->getBodyAsByteArray());

        msg.setMessageType(MQMT_DATAGRAM);
        msg.setPersistence(MQPER_PERSISTENT);

        //        qDebug() << "Write body to message: " << bodyArray->size();// << endl << bodyArray->toHex();
        //msg.useFullBuffer(bodyArray->constData(), bodyArray->size());
        msg.write(bodyArray->size(), bodyArray->constData());

        ImqPutMessageOptions pmo;

        if(!queue.put(msg)) {
            //            ErrorMessage errmsg;
            //            errmsg.setErrorCode(0);
            //            errmsg.setErrorString("ImqQueue::put error");
            //            emit error(message, errmsg);
//            emit error(message, QString("ImqQueue::put error with reason code %1").arg((int)queue.reasonCode()));
            emit rollback(message);
            return;
        }

        delete bodyArray;

        if (rfh2Header)
            delete rfh2Header;

        msg.clearMessage();
    } else {
        //        ErrorMessage errmsg;
        //        errmsg.setErrorCode(0);
        //        errmsg.setErrorString("bodyArray is NULL");
        //        emit error(message, errmsg);
        emit error(message, QString("bodyArray is NULL"));
        emit rollback(message);
        return;
    }

    queue.close();

    emit produced(message);
    inuse = false;
}

QScriptValue WMQProducerCommiterConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new WMQProducerCommiter();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

bool WMQProducerCommiter::initScriptEngine(QScriptEngine &engine)
{
    QScriptValue ctor = engine.newFunction(WMQProducerCommiterConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&WMQProducerCommiter::staticMetaObject, ctor);
    engine.globalObject().setProperty("WMQProducerCommiter", metaObject);
    return true;
}

void WMQProducerCommiter::commit(Message *msg)
{
//    qDebug() << "WMQProducerCommiter::commit : " << msg.getHeaders().value("FileName");
//    msg.setHeader("emiter", "WMQProducerCommiter");
    emit commited(msg);
}

void WMQProducerCommiter::rollback(Message *msg)
{
//    qDebug() << "WMQProducerCommiter::rollback";
//    msg.setHeader("emiter", "WMQProducerCommiter");
    emit rollbacked(msg);
}
