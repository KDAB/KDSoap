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

//void EndpointReference::loadXML( ParserContext *context, const QDomElement &element )
//{
//    qDebug() << "EndpointReference::loadXML !";
//    QDomElement child = element.firstChildElement();

//    while ( !child.isNull() ) {
//      NSManager namespaceManager( context, child );
//      const QName tagName( child.tagName() );
//      if ( tagName.localName() == QLatin1String("Address") ) {
//          m_address = child.text();
//      } else if ( tagName.localName() == QLatin1String("ReferenceParameters")) {
//            // how to handle this data ? any object of any namespace...
//      } else if ( tagName.localName() == QLatin1String("Metadata")) {
//            // how to handle this data ? any object of any namespace...
//      } else {
//        context->messageHandler()->warning( QString::fromLatin1("EndPointReference: unknown tag %1" ).arg( child.tagName() ) );
//      }

//      child = child.nextSiblingElement();
//    }
//    if (m_address.isEmpty())
//        context->messageHandler()->warning( QLatin1String("EndPointReference: 'Address' required") );
//}

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

