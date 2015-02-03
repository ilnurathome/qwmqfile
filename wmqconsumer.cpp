#include <QDebug>
#include <QXmlStreamReader>
#include "wmqconsumer.h"
#include "iconnection.h"
#include <cmqpsc.h>


QString WMQConsumer::getQueueName() const
{
    return queueName;
}

void WMQConsumer::run()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!connection)
        connection = connectionFactory->getConnection();

    if (!connection) {
        qCritical() << "WMQConsumer::run: can't get connection";
        emit error("can't get connection");
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << ": connection got";

    if (!queue.openStatus()) {
        qDebug() << __PRETTY_FUNCTION__ << ": queue try to open";
        queue.setConnectionReference(((WMQConnection*) connection)->getImqQueueManager());
        queue.setName(queueName.toStdString().c_str());
        queue.setOpenOptions(MQOO_INPUT_AS_Q_DEF | MQOO_FAIL_IF_QUIESCING);
        queue.open();

        if (queue.reasonCode()) {
            qCritical() << "ImqQueue::open ended with reason code " << (int)queue.reasonCode();
            qCritical() << "ImqQueue::open ended with completion code " << (int)queue.completionCode();
            emit error(QString("ImqQueue::open error with reason code %1").arg((int)queue.reasonCode()));
            return;
        }
    }

    qDebug() << __PRETTY_FUNCTION__ << ": queue opened";

    isquit = false;

    while (!isquit) {
        ImqMessage msg;
        ImqGetMessageOptions gmo;


        gmo.setOptions(gmo.options() | MQGMO_WAIT | MQGMO_PROPERTIES_FORCE_MQRFH2);
        gmo.setWaitInterval(5);

        if (transacted) {
            queue.connectionReference()->begin();
            gmo.setOptions(gmo.options() | MQGMO_SYNCPOINT);
        }

        if(queue.get(msg, gmo)) {
            char *pszDataPointer = msg.dataPointer();
            size_t dataLength = msg.dataLength();

            ByteMessage newmsg;

            MQLONG hsize = parseMQRFHeader2(pszDataPointer, dataLength, newmsg.getHeaders());

            /*
             * hsize checks
             */
            if (hsize < 0 || (MQLONG)dataLength - hsize < 0) {
                qCritical() << __PRETTY_FUNCTION__ << ": bad hsize=" << hsize << "; dataLength - hsize=" << dataLength - hsize;
                emit error(QString(__PRETTY_FUNCTION__) + QString(": bad hsize"));
                return;
            }

            // copy data
            //            QByteArray msgData;
            //            msgData.append(pszDataPointer  + hsize, dataLength - hsize);
            //            qDebug() << msgData;
            //            QByteArray msgData = QByteArray::fromRawData(pszDataPointer  + hsize, dataLength - hsize);
            //newmsg->setBody(msgData);
            newmsg.getBodyAsByteArray().append(pszDataPointer  + hsize, dataLength - hsize);

            // set message id
            QByteArray newmsgid;
            newmsgid.append((char*)msg.messageId().dataPointer(), msg.messageId().dataLength());

            newmsg.setMessageId(newmsgid);

            if(msg.correlationId().dataLength() > 0) {
                QByteArray correlid;
                correlid.append((char*)msg.correlationId().dataPointer(), msg.correlationId().dataLength());
                newmsg.setHeader(MESSAGE_CORRELATION_ID, QString(correlid));
            }

            //            qDebug() << newmsg->getBodyAsByteArray() << newmsg->getBodyAsByteArray()->data();


            //            QHashIterator<QString, QString> headerIterator(newmsg->getHeaders());
            //            while (headerIterator.hasNext()) {
            //                headerIterator.next();
            //                QString headerName = headerIterator.key();
            //                QString headerValue = headerIterator.value();
            //                qDebug() << __PRETTY_FUNCTION__ << ": headerName=" << headerName << "; headerValue=" << headerValue;
            //            }

            msgConsumed++;
            newmsg.setHeader("consumerCounter", QString::number(msgConsumed));
            qDebug() << __PRETTY_FUNCTION__ << ": nConsumer=" << nConsumer << "; msgConsumed=" << msgConsumed << "; msgCommited=" << msgCommited << "; msgRollbacked=" << msgRollbacked << "; balance=" << msgConsumed-msgCommited-msgRollbacked;

            emit message(newmsg);
        }
    }
    qDebug() << __PRETTY_FUNCTION__ <<":finish";
}

void WMQConsumer::setQueueName(const QString &value)
{
    queueName = value;
}

int WMQConsumer::init()
{
    return 0;
}

void WMQConsumer::quit()
{
    qDebug() << __PRETTY_FUNCTION__ <<":quit";
    isquit = true;
}

iConnectionFactory *WMQConsumer::getConnectionFactory() const
{
    return connectionFactory;
}

void WMQConsumer::commit(Message &msg)
{
    if (transacted) {
        if(!queue.connectionReference()->commit()) {
            qCritical() << __PRETTY_FUNCTION__ << ":commit ended with reason code " << (int)queue.reasonCode();
            emit error(QString("commit ended with reason code %1").arg((int)queue.reasonCode()));
            return;
        }
    }
    msgCommited++;
}

void WMQConsumer::rollback(Message &msg)
{
    if (transacted) {
        if(!queue.connectionReference()->backout()) {
            qCritical() << __PRETTY_FUNCTION__ << ":backout ended with reason code " << (int)queue.reasonCode();
            emit error(QString("backout ended with reason code %1").arg((int)queue.reasonCode()));
            return;
        }
    }

    msgRollbacked++;
}

void WMQConsumer::setConnectionFactory(iConnectionFactory *value)
{
    connectionFactory = value;
}


int WMQConsumer::getNConsumer() const
{
    return nConsumer;
}

void WMQConsumer::setNConsumer(int value)
{
    nConsumer = value;
}

bool WMQConsumer::getTransacted() const
{
    return transacted;
}

void WMQConsumer::setTransacted(bool value)
{
    transacted = value;
}

WMQConsumer::WMQConsumer(QObject *parent) :
    QObject(parent), connectionFactory(NULL), connection(NULL), msgConsumed(0), msgCommited(0), msgRollbacked(0), transacted(false)
{
    qDebug() << __PRETTY_FUNCTION__;
}

WMQConsumer::~WMQConsumer()
{
    qDebug() << __PRETTY_FUNCTION__;
}


MQLONG parseMQRFHeader2(char *pszDataPointer, size_t len, QHash<QString, QString> &props)
{
    PMQCHAR msgBuf = pszDataPointer;
    PMQRFH2 pRFH2 = (PMQRFH2)msgBuf;                                /* ptr to RFH2               */
    MQINT32 nameValueLength;                      /* RFH2 NameValuelength      */
    PMQCHAR pNameValueData;                       /* ptr to RFH2 NameValeData  */

    if (memcmp(pRFH2->StrucId, MQRFH_STRUC_ID, sizeof(MQINT32)) == 0) {
        //        qDebug() << "parseMQRFHeader2::pRFH2->Version=" << pRFH2->Version;
        if (pRFH2->Version == MQRFH_VERSION_2) {
            //            qDebug() << "parseMQRFHeader2::pRFH2->StrucLength=" << pRFH2->StrucLength;
            if (pRFH2->StrucLength > (int)(MQRFH_STRUC_LENGTH_FIXED_2 + sizeof(MQINT32)) && (size_t)pRFH2->StrucLength <= len)
            {
                pNameValueData = msgBuf + MQRFH_STRUC_LENGTH_FIXED_2 + sizeof(MQINT32);
                QByteArray tba(pNameValueData, pRFH2->StrucLength - sizeof(MQINT32) - MQRFH_STRUC_LENGTH_FIXED_2);
                //                qDebug() << "parseMQRFHeader2::tba=" << tba;

                while (pNameValueData - msgBuf < pRFH2->StrucLength) {
                    /* Don't risk causing a SIGBUS, memcpy the four bytes             */
                    memcpy(&nameValueLength, pNameValueData-4, 4);
                    //                    qDebug() << "parseMQRFHeader2::nameValueLength=" << nameValueLength;

                    //                    qDebug() << "parseMQRFHeader2::pNameValueData=" << (long)pNameValueData << "; pNameValueData - msgBuf" << (long)(pNameValueData - msgBuf);

                    QByteArray ba(pNameValueData, nameValueLength);
                    //                    qDebug() << "parseMQRFHeader2::ba=" << ba;

                    QXmlStreamReader xml(ba);

                    while(xml.readNextStartElement()) {
                        //                        qDebug() << "parseMQRFHeader2::xml element" << xml.name();
                        if (xml.name().compare(MQRFH2_PUBSUB_CMD_FOLDER) == 0) {
                            while(xml.readNextStartElement()) {
                                QString key(MQRFH2_PUBSUB_CMD_FOLDER);
                                key.append("_");
                                key.append(xml.name());
                                props.insert(key, xml.readElementText());
                            }
                        } else if (xml.name().compare(MQRFH2_PUBSUB_RESP_FOLDER) == 0) {
                            while(xml.readNextStartElement()) {
                                QString key(MQRFH2_PUBSUB_RESP_FOLDER);
                                key.append("_");
                                key.append(xml.name());
                                props.insert(key, xml.readElementText());
                            }
                        } else if (xml.name().compare(MQRFH2_MSG_CONTENT_FOLDER) == 0) {
                            while(xml.readNextStartElement()) {
                                QString key(MQRFH2_MSG_CONTENT_FOLDER);
                                key.append("_");
                                key.append(xml.name());
                                props.insert(key, xml.readElementText());
                            }
                        } else if (xml.name().compare(MQRFH2_USER_FOLDER) == 0) {
                            while(xml.readNextStartElement()) {
                                QString key;
                                key.append(xml.name());
                                props.insert(key, xml.readElementText());
                            }
                        } else {
                            xml.skipCurrentElement();
                        }
                    }

                    pNameValueData += nameValueLength;
                }
            }
        }
        return pRFH2->StrucLength;
    }
    return 0;
}
