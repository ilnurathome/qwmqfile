#ifndef PRODUCERCONTROLLER_H
#define PRODUCERCONTROLLER_H

#include <QObject>

class ProducerController : public QObject
{
    Q_OBJECT

    QThread
public:
    explicit ProducerController(QObject *parent = 0);

signals:

public slots:

};

#endif // PRODUCERCONTROLLER_H
