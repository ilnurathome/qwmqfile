#include "connectionpool.h"
#include <QDebug>


QScriptValue connectionPoolConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new ConnectionPool();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}


bool ConnectionPool::initScriptEngine(QScriptEngine &engine)
{
        QScriptValue ctor = engine.newFunction(connectionPoolConstructor);
        QScriptValue metaObject = engine.newQMetaObject(&ConnectionPool::staticMetaObject, ctor);
        engine.globalObject().setProperty("ConnectionPool", metaObject);
        return true;
}

int ConnectionPool::getMaxConnections() const
{
    return maxConnections;
}

void ConnectionPool::setMaxConnections(int value)
{
    maxConnections = value;
}

iConnectionFactory* ConnectionPool::getConnectionFactory()
{
    return connectionFactory;
}

void ConnectionPool::setConnectionFactory(iConnectionFactory *value)
{
    connectionFactory = value;
}

iConnection *ConnectionPool::getNewConnection()
{
    iConnection* conn = connectionFactory->getConnection();
    if (conn) {
        ConnectionState connState;
        connState.connection=conn;
        connState.state = OCCUPIED;
        connections.insert(conn, connState);
//        qDebug() << "Insert new connection to pool, pool size: " << conn << "; size:" << connections.size();
    }
    return conn;
}

ConnectionPool::ConnectionPool(QObject *parent) :
    QObject(parent)
{
    maxConnections=3;
    connectionFactory=NULL;
}

ConnectionPool::~ConnectionPool()
{
    QHashIterator<iConnection*, ConnectionState> i(connections);
    if (connectionFactory)
        while(i.hasNext()) {
            i.next();
            if (i.value().connection) {
                connectionFactory->releaseConnection(i.value().connection);
//                qDebug() << "Remove connection from pool : " << connections.size();
            }
        }
    connections.clear();
//    qDebug() << "Clear connections pool";
}

iConnection *ConnectionPool::getConnection()
{
    if (!connectionFactory)
        return NULL;

    iConnection* conn = NULL; // return NULL if pool is full

    mutex.lock();
    QMutableHashIterator <iConnection*, ConnectionState> i(connections);
    while(i.hasNext()) {
        i.next();
//        qDebug() << "Connection state in pool : " << i.key() << " ; state: " << i.value().state;
        if (i.value().state == AVAILABLE && i.value().connection) {
//            qDebug() << "Return connection from pool : " << i.key() << " ; size: " << connections.size();
            i.value().state = OCCUPIED;
            conn = i.value().connection;
        }
    }

    if (!conn)
        if (connections.size() < maxConnections)
            conn = getNewConnection();

    mutex.unlock();

//    if (!conn)
//        qDebug() << "Connection pool is FULL return NULL";
    return conn;
}

int ConnectionPool::releaseConnection(iConnection *connection)
{
    mutex.lock();
//    qDebug() << "Try to release connection in pool: " << connection;

    if (connections.contains(connection)) {
        connections[connection].state = AVAILABLE;
//        qDebug() << "Release connection in pool: " << connection << "; size:" << connections.size();
    }

//    QHash<iConnection*, ConnectionState>::iterator i = connections.find(connection);
//    while (i!= connections.end() && i.key() == connection) {
//        i.value().state = AVAILABLE;
//        qDebug() << "Release connection in pool: " << connection << "; size:" << connections.size();
//        ++i;
//    }
    mutex.unlock();
    return 0;
}

