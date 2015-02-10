#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <QObject>
#include <QHash>
#include "message.h"

/**
 * @brief The Exchange class
 * For future release
 */
class Exchange : public QObject
{
    Q_OBJECT

    Message *in;
    Message *out;
public:
    explicit Exchange(QObject *parent = 0);

    Message *getIn() const;
    void setIn(Message *value);

    Message *getOut() const;
    void setOut(Message *value);

signals:

public slots:

};

#endif // EXCHANGE_H
