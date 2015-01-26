#include "exchange.h"


Message Exchange::getIn() const
{
    return in;
}

void Exchange::setIn(const Message &value)
{
    in = value;
}

Message Exchange::getOut() const
{
    return out;
}

void Exchange::setOut(const Message &value)
{
    out = value;
}
Exchange::Exchange(QObject *parent) :
    QObject(parent)
{
}
