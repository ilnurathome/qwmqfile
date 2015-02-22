#include <QDebug>
#include "protonmessengerproxy.h"

Message *proton_message_conv(pn_message_t *message)
{
    Message *newmsg = new Message(QByteArray());

    pn_data_t data = pn_message_properties(message);

    while(pn_data_next(data)) {
        pn_type_t type = pn_data_type(data);

        switch (type) {
        case PN_STRING:
        case PN_SYMBOL:
        {
            pn_bytes_t key = pn_data_get_bytes(data);
            if(pn_data_next(data)) {
                pn_bytes_t value = pn_data_get_bytes(data);
                newmsg->setHeader(
                            QString(QByteArray(key.start, (int)key.size)),
                            QString(QByteArray(value.start, (int)value.size)));
            }
        }
            break;
        default:
            break;
        }
    }

    pn_bytes_t body = pn_data_get_bytes(pn_message_body(message));

    newmsg->setBody(QVariant(QByteArray(body.start, (int)body.size)));

    return newmsg;
}


QString ProtonMessengerProxy::getName() const
{
    return name;
}

void ProtonMessengerProxy::commit(PMessage msg)
{

}

void ProtonMessengerProxy::rollback(PMessage msg)
{

}

void ProtonMessengerProxy::produce(PMessage msg)
{
    pn_message_t * msg;
    msg = pn_message();
    //FIXME

}

void ProtonMessengerProxy::setName(const QString &value)
{
    name = value;
}

void ProtonMessengerProxy::status()
{

}

ProtonMessengerProxy::ProtonMessengerProxy(QObject *parent) :
    QObject(parent), messenger(NULL)
{
}

ProtonMessengerProxy::~ProtonMessengerProxy()
{
}

int ProtonMessengerProxy::init()
{
    messenger = (name.size() == 0)? pn_messenger(NULL) : pn_messenger(name.toAscii().constData());

    if (!messenger) {
        qCritical() << __PRETTY_FUNCTION__ << ": Can't create messenger";
    }

    return 0;
}

void ProtonMessengerProxy::run()
{
    stop = false;

    pn_message_t * msg;
    msg = pn_message();
    //FIXME

    while(!stop) {
        pn_messenger_work(messenger, 5000);

        while(pn_messenger_incoming(messenger)) {
            pn_messenger_get(messenger, msg);
            if(pn_messenger_errno(messenger)) {
                qCritical() << __PRETTY_FUNCTION__ << ": get error";
                emit error("get error");
            }

            Message *newmsg = proton_message_conv(msg);
            emit message(PMessage(newmsg));
        }
    }
}
