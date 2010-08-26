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

void KDSoapMessage::addArgument(const QString &argumentName, const QVariant& argumentValue, const QString& typeNameSpace, const QString& typeName)
{
    d->args.append(KDSoapValue(argumentName, argumentValue, typeNameSpace, typeName));
}

void KDSoapMessage::addArgument(const QString& argumentName, const KDSoapValueList& argumentValueList, const QString& typeNameSpace, const QString& typeName)
{
    d->args.append(KDSoapValue(argumentName, argumentValueList, typeNameSpace, typeName));
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
            .arg(d->args.child(QLatin1String("faultcode")).value().toString())
            .arg(d->args.child(QLatin1String("faultstring")).value().toString())
            .arg(d->args.child(QLatin1String("faultactor")).value().toString());
}

void KDSoapMessage::setFault(bool fault)
{
    d->isFault = fault;
}

KDSoapMessage::Use KDSoapMessage::use() const
{
    return d->use;
}

void KDSoapMessage::setUse(Use use)
{
    d->use = use;
}
