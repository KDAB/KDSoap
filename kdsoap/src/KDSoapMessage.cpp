#include "KDSoapMessage.h"
#include "KDSoapMessage_p.h"
#include <QDebug>
#include <QVariant>

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
    d->args.append(KDSoapValue(argumentName, argumentValue));
}

//KDSoapValueList KDSoapMessage::arguments() const
//{
//    return d->args;
//}

//QVariant KDSoapMessage::argument(const QString &argumentName) const
//{
//    return d->args.value(argumentName);
//}

QDebug operator <<(QDebug dbg, const KDSoapMessage &msg)
{
    KDSoapValueListIterator it(msg.d->args);
    while (it.hasNext()) {
        const KDSoapValue& value = it.next();
        dbg << value.name << value.value;
    }
    return dbg;
}
