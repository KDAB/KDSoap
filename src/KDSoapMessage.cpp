#include "KDSoapMessage.h"
#include <QDebug>
#include <QVariant>

class KDSoapMessageData : public QSharedData
{
public:
    QMap<QString, QVariant> args;
};

KDSoapMessage::KDSoapMessage()
    : d(new KDSoapMessageData)
{
}

KDSoapMessage::KDSoapMessage(const KDSoapMessage& other)
    : d(other.d)
{
}

KDSoapMessage &KDSoapMessage::operator =(const KDSoapMessage &other)
{
    d = other.d;
    return *this;
}

KDSoapMessage::~KDSoapMessage()
{
}

void KDSoapMessage::addArgument(const QString &argumentName, const QVariant& argumentValue)
{
    d->args.insert(argumentName, argumentValue);
}

QMap<QString, QVariant> KDSoapMessage::arguments() const
{
    return d->args;
}
