#include "KDSoapEndpointReference.h"

#include <QDebug>

class KDSoapEndpointReferenceData : public QSharedData
{
public:
    KDSoapEndpointReferenceData() {}

    QString m_address;
    KDSoapValueList m_metadata;
    KDSoapValueList m_referenceParameters;
};

KDSoapEndpointReference::KDSoapEndpointReference(const QString &address)
    : d(new KDSoapEndpointReferenceData)
{
    d->m_address = address;
}

KDSoapEndpointReference::KDSoapEndpointReference(const KDSoapEndpointReference &other)
    : d(other.d)
{
}

KDSoapEndpointReference &KDSoapEndpointReference::operator =(const KDSoapEndpointReference &other)
{
    d = other.d;
    return *this;
}

KDSoapEndpointReference::~KDSoapEndpointReference()
{
}

QString KDSoapEndpointReference::address() const
{
    return d->m_address;
}

void KDSoapEndpointReference::setAddress(const QString &address)
{
    d->m_address = address;
}

KDSoapValueList KDSoapEndpointReference::metadata() const
{
    return d->m_metadata;
}

void KDSoapEndpointReference::setMetadata(const KDSoapValueList &metadata)
{
    d->m_metadata = metadata;
}

bool KDSoapEndpointReference::isEmpty() const
{
    return d->m_address.isEmpty();
}

KDSoapValueList KDSoapEndpointReference::referenceParameters() const
{
    return d->m_referenceParameters;
}

void KDSoapEndpointReference::setReferenceParameters(const KDSoapValueList &referenceParameters)
{
    d->m_referenceParameters = referenceParameters;
}

