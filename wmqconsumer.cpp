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
    qDebug() << "WMQConsumer::run";
    if (!connection)
        connection = connectionFactory->getConnection();

    if (!connection) {
        qCritical() << "WMQConsumer::run: can't get connection";
        emit error("can't get connection");
        return;
    }

    qDebug() << "WMQConsumer::run: connection got";

    if (!queue.openStatus()) {
        qDebug() << "WMQConsumer::run: queue try to open";
        queue.setConnectionReference(((WMQConnection*) connection)->getImqQueueManager());
        queue.setName(queueName.toStdString().c_str());
        queue.setOpenOptions(MQOO_INPUT_AS_Q_DEF | MQOO_FAIL_IF_QUIESCING);
        queue.open();

        if (queue.reasonCode()) {
            qCritical() << "ImqQueue::open ended with reason code " << (int)queue.reasonCode();
            qCritical() << "ImqQueue::open ended with reason code " << (int)queue.completionCode();
            emit error(QString("ImqQueue::open error with reason code %1").arg((int)queue.reasonCode()));
            return;
        }
    }

    qDebug() << "WMQConsumer::run: queue opened";

    while (true) {
        ImqMessage msg;
        ImqGetMessageOptions gmo;

        gmo.setOptions(gmo.options() | MQGMO_PROPERTIES_FORCE_MQRFH2);

        if(queue.get(msg, gmo)) {
            char *pszDataPointer = msg.dataPointer();
            int dataLength = msg.dataLength();

            ByteMessage *newmsg = new ByteMessage();

            int hsize = parseMQRFHeader2(pszDataPointer, newmsg->getHeaders());

            QByteArray msgData;
            msgData.append(pszDataPointer  + hsize, dataLength - hsize);
//            qDebug() << msgData;
//            QByteArray msgData = QByteArray::fromRawData(pszDataPointer  + hsize, dataLength - hsize);
            newmsg->setBody(msgData);

//            qDebug() << newmsg->getBodyAsByteArray() << newmsg->getBodyAsByteArray()->data();


            QHashIterator<QString, QString> headerIterator(newmsg->getHeaders());
            while (headerIterator.hasNext()) {
                headerIterator.next();
                QString headerName = headerIterator.key();
                QString headerValue = headerIterator.value();
                qDebug() << "void WMQConsumer::run: headerName=" << headerName << "; headerValue=" << headerValue;
            }

            msgCounter++;
            newmsg->setHeader("consumerCounter", QString::number(msgCounter));
            qDebug() << "consumerCounter: " << msgCounter;

            emit message(newmsg);
        }
    }
}

void WMQConsumer::setQueueName(const QString &value)
{
    queueName = value;
}

void WMQConsumer::init()
{

}

iConnectionFactory *WMQConsumer::getConnectionFactory() const
{
    return connectionFactory;
}

void WMQConsumer::commit(Message *msg)
{
    if (msg) delete msg;
}

void WMQConsumer::rollback(Message *msg)
{
    if (msg) delete msg;
}

void WMQConsumer::setConnectionFactory(iConnectionFactory *value)
{
    connectionFactory = value;
}
WMQConsumer::WMQConsumer(QObject *parent) :
    QObject(parent), connection(NULL), msgCounter(0), connectionFactory(NULL)
{
}


int parseMQRFHeader2(char *pszDataPointer, QHash<QString, QString> &props)
{
    PMQCHAR msgBuf = pszDataPointer;
    PMQRFH2 pRFH2 = (PMQRFH2)msgBuf;                                /* ptr to RFH2               */
    MQINT32 nameValueLength;                      /* RFH2 NameValuelength      */
    PMQCHAR pNameValueData;                       /* ptr to RFH2 NameValeData  */
    bool foundFolder=false;                       /* found folder              */

    if (memcmp(pRFH2->StrucId, MQRFH_STRUC_ID, sizeof(MQINT32)) == 0) {
        qDebug() << "parseMQRFHeader2::pRFH2->Version=" << pRFH2->Version;
        if (pRFH2->Version == MQRFH_VERSION_2) {
            qDebug() << "parseMQRFHeader2::pRFH2->StrucLength=" << pRFH2->StrucLength;
            if (pRFH2->StrucLength > MQRFH_STRUC_LENGTH_FIXED_2 + sizeof(MQINT32))
            {
                pNameValueData = msgBuf + MQRFH_STRUC_LENGTH_FIXED_2 + sizeof(MQINT32);
                QByteArray tba(pNameValueData, pRFH2->StrucLength - sizeof(MQINT32) - MQRFH_STRUC_LENGTH_FIXED_2);
                qDebug() << "parseMQRFHeader2::tba=" << tba;

                while (pNameValueData - msgBuf < pRFH2->StrucLength) {
                    /* Don't risk causing a SIGBUS, memcpy the four bytes             */
                    memcpy(&nameValueLength, pNameValueData-4, 4);
                    qDebug() << "parseMQRFHeader2::nameValueLength=" << nameValueLength;

                    qDebug() << "parseMQRFHeader2::pNameValueData=" << (long)pNameValueData << "; pNameValueData - msgBuf" << (long)(pNameValueData - msgBuf);

                    QByteArray ba(pNameValueData, nameValueLength);
                    qDebug() << "parseMQRFHeader2::ba=" << ba;

                    QXmlStreamReader xml(ba);

                    while(xml.readNextStartElement()) {
                        qDebug() << "parseMQRFHeader2::xml element" << xml.name();
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
