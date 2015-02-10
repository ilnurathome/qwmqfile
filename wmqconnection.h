#ifndef WMQCONNECTION_H
#define WMQCONNECTION_H

#include <imqi.hpp> // WebSphere MQ MQI

#include <QtCore>
#include <QCoreApplication>
#include <QScriptEngine>

#include "iconnection.h"

/**
 * @brief The WMQConnection class
 * Connection to WMQ
 */
class WMQConnection : public iConnection
{
    ImqQueueManager *mgr;
    ImqChannel * pchannel;

    QString channelName;
    int transportType;
    QString connectionName;
    QString queueManagerName;

public:
    WMQConnection();
    ~WMQConnection();

    virtual int open();
    virtual int close();

    ImqQueueManager* getImqQueueManager();

    /**
      * FIXME! Need session class and commit and rollback must be there
      */
    int begin();
    int commit();
    int rollback();

    QString getChannelName() const;
    void setChannelName(const QString &value);
    int getTransportType() const;
    void setTransportType(int value);
    QString getConnectionName() const;
    void setConnectionName(const QString &value);
    QString getQueueManagerName() const;
    void setQueueManagerName(const QString &value);
};

class WMQConnectionFactory : public QObject, public iConnectionFactory
{
    Q_OBJECT

    QString channelName;
    int transportType;
    QString connectionName;
    QString queueManagerName;

public:
    WMQConnectionFactory();
    virtual iConnection *getConnection();
    virtual int releaseConnection(iConnection* connection);

public slots:
    static bool initScriptEngine(QScriptEngine &engine);

    QString getQueueManagerName() const;
    void setQueueManagerName(const QString &value);
    QString getConnectionName() const;
    void setConnectionName(const QString &value);
    int getTransportType() const;
    void setTransportType(int value);
    QString getChannelName() const;
    void setChannelName(const QString &value);
};

#endif // WMQCONNECTION_H
