//#include <QMessageLogger>
#include "wmqconnection.h"

QScriptValue wmqConnectionFactoryConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new WMQConnectionFactory();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}


bool WMQConnectionFactory::initScriptEngine(QScriptEngine &engine)
{
    QScriptValue ctor = engine.newFunction(wmqConnectionFactoryConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&WMQConnectionFactory::staticMetaObject, ctor);
    engine.globalObject().setProperty("WMQConnectionFactory", metaObject);
    return true;
}



QString WMQConnection::getChannelName() const
{
    return channelName;
}

void WMQConnection::setChannelName(const QString &value)
{
    channelName = value;
}

int WMQConnection::getTransportType() const
{
    return transportType;
}

void WMQConnection::setTransportType(int value)
{
    transportType = value;
}

QString WMQConnection::getConnectionName() const
{
    return connectionName;
}

void WMQConnection::setConnectionName(const QString &value)
{
    connectionName = value;
}

QString WMQConnection::getQueueManagerName() const
{
    return queueManagerName;
}

void WMQConnection::setQueueManagerName(const QString &value)
{
    queueManagerName = value;
}
WMQConnection::WMQConnection()
{
    channelName = "JAVA.CHANNEL";
    transportType = MQXPT_TCP;
    connectionName = "devmod01v.corp.dev.vtb(1414)";
    queueManagerName = "TEST.QM" ;
    mgr = NULL;
}

WMQConnection::~WMQConnection()
{
    close();
    if (mgr)
        delete mgr;
}

int WMQConnection::open()
{
    if (!mgr)
        mgr = new ImqQueueManager();

    pchannel = new ImqChannel;
    if (pchannel == NULL) {
        qCritical() << "Can't create ImqChannel";
        return -1;
    }

    pchannel->setHeartBeatInterval(1);
    pchannel->setChannelName(channelName.toStdString().c_str());
    pchannel->setTransportType(transportType);
    pchannel->setConnectionName(connectionName.toStdString().c_str());

    mgr->setChannelReference(pchannel);
    mgr->setName(queueManagerName.toStdString().c_str());
    mgr->setOpenOptions(mgr->openOptions() | MQCNO_HANDLE_SHARE_BLOCK | MQCNO_RECONNECT_Q_MGR);

    if (!mgr->connect()) {
        qCritical() << "ImqQueueManager::connect ended with reason code " << (int)mgr->reasonCode();
        return -1;
    }

//    qDebug() << __PRETTY_FUNCTION__ << ":MQR open options: " << hex << (mgr->openOptions() & MQCNO_HANDLE_SHARE_BLOCK);

    return 0;
}

int WMQConnection::close()
{
    if (!mgr) return -1;
    // Disconnect from MQM if not already connected (the
    // ImqQueueManager object handles this situation automatically)
    if ( ! mgr->disconnect( ) ) {
        /* report reason, if any   */
        qCritical() << "ImqQueueManager::disconnect ended with reason code " << (int)mgr->reasonCode();
        return -1;
    }

    mgr->setChannelReference();

    // Tidy up the channel object if allocated.
    if ( pchannel ) {
        delete pchannel;
        pchannel = NULL;
    }
    return 0;
}


ImqQueueManager* WMQConnection::getImqQueueManager()
{
    return mgr;
}

int WMQConnection::begin()
{
    mgr->begin();
    return 0;
}

int WMQConnection::commit()
{
    mgr->commit();
    return 0;
}

int WMQConnection::rollback()
{
    mgr->backout();
    return 0;
}

int WMQConnectionFactory::releaseConnection(iConnection *connection)
{
    if(connection) {
        connection->close();
        delete connection;
    }
    return 0;
}

QString WMQConnectionFactory::getQueueManagerName() const
{
    return queueManagerName;
}

void WMQConnectionFactory::setQueueManagerName(const QString &value)
{
    queueManagerName = value;
}

QString WMQConnectionFactory::getConnectionName() const
{
    return connectionName;
}

void WMQConnectionFactory::setConnectionName(const QString &value)
{
    connectionName = value;
}

int WMQConnectionFactory::getTransportType() const
{
    return transportType;
}

void WMQConnectionFactory::setTransportType(int value)
{
    transportType = value;
}

QString WMQConnectionFactory::getChannelName() const
{
    return channelName;
}

void WMQConnectionFactory::setChannelName(const QString &value)
{
    channelName = value;
}
WMQConnectionFactory::WMQConnectionFactory(QObject *parent) :
    QObject(parent), transportType(MQXPT_TCP)
{
}

iConnection *WMQConnectionFactory::getConnection()
{
    WMQConnection* conn = new WMQConnection();

    if(channelName.size())
        conn->setChannelName(channelName);

    if(connectionName.size())
        conn->setConnectionName(connectionName);

    if(queueManagerName.size())
        conn->setQueueManagerName(queueManagerName);

    if(transportType)
        conn->setTransportType(transportType);

    conn->open();
    return (iConnection*)conn;
}
