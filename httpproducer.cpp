#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>
#include "httpproducer.h"
#include "bytemessage.h"

QScriptValue HTTPProducerConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new HTTPProducer();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

bool HTTPProducer::initScriptEngine(QScriptEngine &engine)
{
    QScriptValue ctor = engine.newFunction(HTTPProducerConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&HTTPProducer::staticMetaObject, ctor);
    engine.globalObject().setProperty("HTTPProducer", metaObject);
    return true;
}

void HTTPProducer::produce(PMessage message)
{
    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // the HTTP request
    QNetworkRequest req;

    req.setHeader(QNetworkRequest::ContentTypeHeader, QString("text/xml;charset=utf-8"));
    req.setUrl((QUrl( QString("http://cbr.ru/secinfo/secinfo.asmx") ) ));

//    qDebug() << "req.header:" << req.header(QNetworkRequest::ContentTypeHeader);

    QByteArray data = message.data()->getBodyAsByteArray();
    //        QByteArray data = QString("<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:web=\"http://web.cbr.ru/\"><soapenv:Header/><soapenv:Body><web:IDRepo><web:OnDate>2015-02-03</web:OnDate></web:IDRepo></soapenv:Body></soapenv:Envelope>").toAscii();

    req.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(qulonglong(data.size())));

    QNetworkReply *reply = mgr.post(req, data);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
        //success
                    qDebug() << "Success" << reply->readAll();

        Message *retmsg = new Message(reply->readAll());

        emit produced(PMessage(retmsg));

        // NB! temp
        emit commited(message);
        delete reply;
    } else {
        //failure
//        qDebug() << "Failure" <<reply->errorString();
//        qDebug() << "reply.header:" << reply->header(QNetworkRequest::ContentTypeHeader);
//        qDebug() << reply->readAll();

        // NB! temp
        emit rollbacked(message);
        delete reply;
    }

}

HTTPProducer::HTTPProducer(QObject *parent) :
    QObject(parent)
{
}
