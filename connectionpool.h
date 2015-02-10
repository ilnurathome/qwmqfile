#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QScriptEngine>

#include "iconnection.h"

enum ConnectionStates {OCCUPIED, AVAILABLE};

/**
 * @brief The ConnectionPool class
 * NB! Must releaseConnection after use when not required
 */
class ConnectionPool : public QObject, iConnectionFactory
{
    Q_OBJECT

    // For multithreading model
    QMutex mutex;

    class ConnectionState {
    public:
        iConnection* connection;
        ConnectionStates state;
    };

    // FIXME change iConnection* to QSharedPointer<iConnection>
    QHash<iConnection*, ConnectionState> connections;
    int maxConnections;

    // FIXME
    iConnectionFactory* connectionFactory;

    iConnection* getNewConnection();
public:
    explicit ConnectionPool(QObject *parent = 0);
    ~ConnectionPool();

    iConnection *getConnection();

    int getMaxConnections() const;

    iConnectionFactory* getConnectionFactory();

    static bool initScriptEngine(QScriptEngine &engine);

signals:
    void connected(iConnection* connection);

public slots:

    int releaseConnection(iConnection* connection);
    void setConnectionFactory(iConnectionFactory *value);
    void setMaxConnections(int value);
};

#endif // WMQCONNECTIONPOOL_H
