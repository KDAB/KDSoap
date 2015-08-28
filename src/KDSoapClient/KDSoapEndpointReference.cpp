#include "KDSoapEndpointReference.h"

#include <QDebug>


KDSoapEndpointReference::KDSoapEndpointReference()
{
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
    return m_address;
}

void KDSoapEndpointReference::setAddress(const QString &address)
{
    m_address = address;
}
