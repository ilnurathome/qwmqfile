#ifndef ICONNECTION_H
#define ICONNECTION_H

class iConnection
{
public:
    virtual ~iConnection(){}

    /**
     * @brief open
     * @return int
     * If no errors return 0
     * else return <> 0
     */
    virtual int open()=0;

    /**
     * @brief open
     * @return int
     * If no errors return 0
     * else return <> 0
     */
    virtual int close()=0;
};

class iConnectionFactory
{
public:
    virtual ~iConnectionFactory(){}

    virtual iConnection* getConnection()=0;
    virtual int releaseConnection(iConnection* connection)=0;
};

#endif // ICONNECTION_H
