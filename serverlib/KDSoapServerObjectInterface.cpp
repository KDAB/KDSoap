#include "KDSoapServerObjectInterface.h"

class KDSoapServerObjectInterface::Private
{
public:
    KDSoapHeaders m_headers;
    QString m_faultCode;
    QString m_faultString;
    QString m_faultActor;
    QString m_detail;
};

KDSoapServerObjectInterface::KDSoapServerObjectInterface()
    : d(new Private)
{
}

KDSoapServerObjectInterface::~KDSoapServerObjectInterface()
{
    delete d;
}

void KDSoapServerObjectInterface::setFault(const QString &faultCode, const QString &faultString, const QString &faultActor, const QString &detail)
{
    Q_ASSERT(!faultCode.isEmpty());
    d->m_faultCode = faultCode;
    d->m_faultString = faultString;
    d->m_faultActor = faultActor;
    d->m_detail = detail;
}

void KDSoapServerObjectInterface::resetFault()
{
    d->m_faultCode.clear();
}

void KDSoapServerObjectInterface::storeFaultAttributes(KDSoapMessage& message) const
{
    message.addArgument(QString::fromLatin1("faultcode"), d->m_faultCode);
    message.addArgument(QString::fromLatin1("faultstring"), d->m_faultString);
    message.addArgument(QString::fromLatin1("faultactor"), d->m_faultActor);
    message.addArgument(QString::fromLatin1("detail"), d->m_detail);
}

bool KDSoapServerObjectInterface::hasFault() const
{
    return !d->m_faultCode.isEmpty();
}

KDSoapHeaders KDSoapServerObjectInterface::headers() const
{
    return d->m_headers;
}

void KDSoapServerObjectInterface::setHeaders(const KDSoapHeaders &headers)
{
    d->m_headers = headers;
}
