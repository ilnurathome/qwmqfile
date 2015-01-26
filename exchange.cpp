#include "exchange.h"


Message *Exchange::getIn() const
{
    return in;
}

void Exchange::setIn(Message *value)
{
    in = value;
}

Message *Exchange::getOut() const
{
    return out;
}

void Exchange::setOut(Message *value)
{
    out = value;
}
Exchange::Exchange(QObject *parent) :
    QObject(parent)
{
}
