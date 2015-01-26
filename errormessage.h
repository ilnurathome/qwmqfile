#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

#include <QObject>


class ErrorMessage : public QObject
{
    Q_OBJECT

    long errorCode;
    QString errorString;

public:
    explicit ErrorMessage(QObject* parent = 0);

    long getErrorCode() const;
    void setErrorCode(long value);

    QString getErrorString() const;
    void setErrorString(const QString &value);

signals:

public slots:

};

#endif // ERRORMESSAGE_H
