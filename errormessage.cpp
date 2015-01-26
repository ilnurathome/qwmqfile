#include "errormessage.h"


long ErrorMessage::getErrorCode() const
{
    return errorCode;
}

void ErrorMessage::setErrorCode(long value)
{
    errorCode = value;
}

QString ErrorMessage::getErrorString() const
{
    return errorString;
}

void ErrorMessage::setErrorString(const QString &value)
{
    errorString = value;
}

ErrorMessage::ErrorMessage(QObject *parent) :
    QObject(parent)
{
}
