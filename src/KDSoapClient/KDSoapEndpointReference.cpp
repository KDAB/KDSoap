#include "KDSoapEndpointReference.h"

#include <QDebug>

class KDSoapEndpointReferenceData : public QSharedData
{
public:
    QString m_address;
    KDSoapValue m_referenceParameters;
    KDSoapValue m_metadata;

//    <wsa:EndpointReference>
//        <wsa:Address>xs:anyURI</wsa:Address>
//        <wsa:ReferenceParameters>xs:any*</wsa:ReferenceParameters> ?
//        <wsa:Metadata>xs:any*</wsa:Metadata>?
//    </wsa:EndpointReference>
    KDSoapEndpointReferenceData() {}
};

KDSoapEndpointReference::KDSoapEndpointReference()
    : d(new KDSoapEndpointReferenceData)
{
}

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
KDSoapValue KDSoapEndpointReference::metadata() const
{
    return d->m_metadata;
}

void KDSoapEndpointReference::setMetadata(const KDSoapValue &metadata)
{
    d->m_metadata = metadata;
}

bool KDSoapEndpointReference::isEmpty() const
{
    return d->m_address.isEmpty();
}

KDSoapValue KDSoapEndpointReference::referenceParameters() const
{
    return d->m_referenceParameters;
}

void KDSoapEndpointReference::setReferenceParameters(const KDSoapValue &referenceParameters)
{
    d->m_referenceParameters = referenceParameters;
}

