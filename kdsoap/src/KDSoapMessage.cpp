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

KDSoapValueList& KDSoapMessage::arguments()
{
    return d->args;
}

const KDSoapValueList& KDSoapMessage::arguments() const
{
    return d->args;
}

QDebug operator <<(QDebug dbg, const KDSoapMessage &msg)
{
    KDSoapValueListIterator it(msg.d->args);
    while (it.hasNext()) {
        const KDSoapValue& value = it.next();
        dbg << value.name() << value.value();
    }
    return dbg;
}

bool KDSoapMessage::isFault() const
{
    return d->isFault;
}

QString KDSoapMessage::faultAsString() const
{
    return QObject::tr("Fault code: %1\nFault description: %2 (%3)")
            .arg(d->args.value(QLatin1String("faultcode")).toString())
            .arg(d->args.value(QLatin1String("faultstring")).toString())
            .arg(d->args.value(QLatin1String("faultactor")).toString());
}

void KDSoapMessage::setFault(bool fault)
{
    d->isFault = fault;
}
