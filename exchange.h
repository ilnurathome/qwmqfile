#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <QObject>
#include <QHash>
#include "message.h"

class Exchange : public QObject
{
    Q_OBJECT

    Message in;
    Message out;
public:
    explicit Exchange(QObject *parent = 0);

    Message getIn() const;
    void setIn(const Message &value);

    Message getOut() const;
    void setOut(const Message &value);

signals:

public slots:

};

#endif // EXCHANGE_H
