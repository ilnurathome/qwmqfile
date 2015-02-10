#include "file2bytearrayprocess.h"
#include <QDebug>
#include <typeinfo>

File2ByteArrayProcess::File2ByteArrayProcess(QObject *parent) : QObject(parent), requestReply(true)
{

}

void File2ByteArrayProcess::process(PMessage message)
{
    try {
        if (typeid(*message.data()) != typeid(FileMessage)) {
            qWarning() << __PRETTY_FUNCTION__ << ": message is not FileMessage it's " << typeid(*message.data()).name() << "; must be " << typeid(FileMessage).name();
            emit error(message, "message is not FileMessage");
            emit rollback(message);
            return;
        }
    }
    catch(...)
    {

    }

    FileMessage *msg = (FileMessage*) message.data();

    QByteArray b;
    QFile *f = msg->getFile();
    if (f) {
        if(f->open(QIODevice::ReadOnly)) {
            b.append(f->readAll());
            f->close();
        }
    }

    //        if (requestReply) {
    //            msgs.insert((Message*)&retmsg,(Message*)&msg);
    //        }

    emit proceed(PMessage(new Message(QVariant(b))));
}

void File2ByteArrayProcess::commit(PMessage msg)
{
    if (requestReply) {
        if (msgs.contains(msg.data())) {
            emit commited(PMessage(msgs.value(msg.data())));
            msgs.remove(msg.data());
        }
    }
}

void File2ByteArrayProcess::rollback(PMessage msg)
{
    if (requestReply) {
        if (msgs.contains(msg.data())) {
            emit commited(PMessage(msgs.value(msg.data())));
            msgs.remove(msg.data());
        }
    }
}



TFile2ByteArrayProcess::TFile2ByteArrayProcess(QObject *parent) : QObject(parent), requestReply(true)
{

}

void TFile2ByteArrayProcess::process(TMessage<QFile> *msg)
{
    QByteArray ba;

    QFile &f = msg->getBody();
    if(f.open(QIODevice::ReadOnly)) {
        ba.append(f.readAll());
        f.close();
    }

    TMessage<QByteArray> *retmsg = new TMessage<QByteArray>();

    if (retmsg) {
        retmsg->getBody().append(ba);
        if (requestReply) {
            msgs.insert(retmsg,msg);
        }

        emit proceed(retmsg);
    } else {
        emit rollback(retmsg);
    }
}

void TFile2ByteArrayProcess::commit(TMessage<QByteArray> *msg)
{
    if (msg) {
        if (requestReply) {
            if (msgs.contains(msg)) {
                emit commited(msgs.value(msg));
            }
        }

        delete msg;
    }
}

void TFile2ByteArrayProcess::rollback(TMessage<QByteArray> *msg)
{
    if (msg) {
        if (requestReply) {
            if (msgs.contains(msg)) {
                emit rollbacked(msgs.value(msg));
            }
        }

        delete msg;
    }
}
