/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>

#include <QDebug>
#include "soapbinding.h"

using namespace KWSDL;

// TODO: set namespace correctly

static QString soapBindingTransportHTTP = QLatin1String("http://schemas.xmlsoap.org/soap/http");
static QString soapStandardNamespace = QLatin1String("http://schemas.xmlsoap.org/wsdl/soap/");
static QString soapEncStandardNamespace = QLatin1String("http://schemas.xmlsoap.org/soap/encoding/");

static QString soapPrefix( ParserContext *context )
{
  QString prefix = context->namespaceManager()->prefix( soapStandardNamespace );
  if ( !prefix.isEmpty() )
    return prefix + QLatin1Char(':');
  else
    return QString();
}

SoapBinding::Binding::Binding()
  : mTransport( HTTPTransport ),
    mStyle( DocumentStyle )
{
}

SoapBinding::Binding::~Binding()
{
}

void SoapBinding::Binding::setTransport( Transport transport )
{
  mTransport = transport;
}

SoapBinding::Transport SoapBinding::Binding::transport() const
{
  return mTransport;
}

void SoapBinding::Binding::setStyle( Style style )
{
  mStyle = style;
}

SoapBinding::Style SoapBinding::Binding::style() const
{
  return mStyle;
}

void SoapBinding::Binding::loadXML( ParserContext *context, const QDomElement &element )
{
  if ( element.hasAttribute( QLatin1String("transport") ) ) {
    if ( element.attribute( QLatin1String("transport") ) == soapBindingTransportHTTP ) {
      mTransport = HTTPTransport;
    } else
      context->messageHandler()->warning( QLatin1String("SoapBinding::Binding: unknown 'transport' value") );
  }

  if ( element.hasAttribute( QLatin1String("style" )) ) {
    if ( element.attribute( QLatin1String("style") ) == QLatin1String("rpc") )
      mStyle = RPCStyle;
    else if ( element.attribute( QLatin1String("style") ) == QLatin1String("document") )
      mStyle = DocumentStyle;
    else
      context->messageHandler()->warning( QLatin1String("SoapBinding::Binding: unknown 'style' value") );
  }
}

void SoapBinding::Binding::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElementNS( soapStandardNamespace, soapPrefix( context ) + QLatin1String("binding") );
  parent.appendChild( element );

  if ( mTransport == HTTPTransport )
    element.setAttribute(QLatin1String( "transport"), soapBindingTransportHTTP );
  else
    context->messageHandler()->warning( QLatin1String("SoapBinding::Binding: unknown 'transport' value") );

  if ( mStyle == RPCStyle )
    element.setAttribute( QLatin1String("style"), QLatin1String("rpc") );
  else
    element.setAttribute( QLatin1String("style"), QLatin1String("document") );
}


SoapBinding::Operation::Operation()
  : mStyle( DocumentStyle )
{
}

SoapBinding::Operation::Operation( const QString &name )
  : mName( name ), mStyle( DocumentStyle )
{
}

SoapBinding::Operation::~Operation()
{
}

void SoapBinding::Operation::loadXML( ParserContext *context, const QDomElement &element )
{
  // read soapAction, discarding leading/trailing space (https://github.com/KDAB/KDSoap/issues/71)
  mSoapAction = element.attribute( QLatin1String("soapAction") ).trimmed();

  //qDebug() << "style=" << element.attribute("style");

  if ( element.hasAttribute( QLatin1String("style") ) ) {
    if ( element.attribute( QLatin1String("style") ) == QLatin1String("rpc") )
      mStyle = RPCStyle;
    else if ( element.attribute( QLatin1String("style") ) == QLatin1String("document") )
      mStyle = DocumentStyle;
    else
      context->messageHandler()->warning( QLatin1String("SoapBinding::Operation: unknown 'style' value") );
  }

  // TODO: fallback is style value of soap:binding
}

void SoapBinding::Operation::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElementNS( soapStandardNamespace, soapPrefix( context ) + QLatin1String("operation") );
  parent.appendChild( element );

  if ( !mSoapAction.isEmpty() )
    element.setAttribute( QLatin1String("soapAction"), mSoapAction );

  if ( mStyle == RPCStyle )
    element.setAttribute( QLatin1String("style"), QLatin1String("rpc") );
  else
    element.setAttribute( QLatin1String("style"), QLatin1String("document") );
}

void SoapBinding::Operation::setName( const QString &name )
{
  mName = name;
}

QString SoapBinding::Operation::name() const
{
  return mName;
}

void SoapBinding::Operation::setAction( const QString &action )
{
  mSoapAction = action;
}

QString SoapBinding::Operation::action() const
{
  return mSoapAction;
}

void SoapBinding::Operation::setStyle( Style style )
{
  mStyle = style;
}

SoapBinding::Style SoapBinding::Operation::style() const
{
  return mStyle;
}

void SoapBinding::Operation::setInput( const Body &input )
{
  mInputBody = input;
}

SoapBinding::Body SoapBinding::Operation::input() const
{
  return mInputBody;
}

void SoapBinding::Operation::setOutput( const Body &output )
{
  mOutputBody = output;
}

SoapBinding::Body SoapBinding::Operation::output() const
{
  return mOutputBody;
}

void SoapBinding::Operation::addInputHeader( const Header &inputHeader )
{
  mInputHeaders << inputHeader;
}

SoapBinding::Headers SoapBinding::Operation::inputHeaders() const
{
  return mInputHeaders;
}

void SoapBinding::Operation::addOutputHeader( const Header &outputHeader )
{
  mOutputHeaders << outputHeader;
}

SoapBinding::Headers SoapBinding::Operation::outputHeaders() const
{
  return mOutputHeaders;
}

void SoapBinding::Operation::setFault( const Fault &fault )
{
  mFault = fault;
}

SoapBinding::Fault SoapBinding::Operation::fault() const
{
  return mFault;
}

SoapBinding::Body::Body()
  : mUse( LiteralUse )
{
}

SoapBinding::Body::~Body()
{
}

#if 0
void SoapBinding::Body::setEncodingStyle( const QString &encodingStyle )
{
  mEncodingStyle = encodingStyle;
}

QString SoapBinding::Body::encodingStyle() const
{
  return mEncodingStyle;
}

void SoapBinding::Body::setPart( const QString &part )
{
  mPart = part;
}

void SoapBinding::Body::setUse( Use use )
{
  mUse = use;
}

void SoapBinding::Body::setNameSpace( const QString &nameSpace )
{
  mNameSpace = nameSpace;
}
#endif

QString SoapBinding::Body::part() const
{
  return mPart;
}

SoapBinding::Use SoapBinding::Body::use() const
{
  return mUse;
}

QString SoapBinding::Body::nameSpace() const
{
  return mNameSpace;
}

void SoapBinding::Body::loadXML( ParserContext *context, const QDomElement &element )
{
#if 0
  // ## Seems to be set to a namespace, always http://schemas.xmlsoap.org/soap/encoding/ ... unused here.
  if ( element.hasAttribute( "encodingStyle" ) )
    mEncodingStyle = element.attribute( "encodingStyle" );
  else
    mEncodingStyle = QString();
#endif

  mPart = element.attribute( QLatin1String("parts") );
  mNameSpace = element.attribute( QLatin1String("namespace") );

  if ( element.hasAttribute( QLatin1String("use") ) ) {
    if ( element.attribute( QLatin1String("use") ) == QLatin1String("literal") )
      mUse = LiteralUse;
    else if ( element.attribute( QLatin1String("use") ) == QLatin1String("encoded") )
      mUse = EncodedUse;
    else
      context->messageHandler()->warning( QLatin1String("SoapBinding::Body: unknown 'use' value") );
  }
}

void SoapBinding::Body::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElementNS( soapStandardNamespace, soapPrefix( context ) + QLatin1String("body") );
  parent.appendChild( element );

#if 0
  if ( !mEncodingStyle.isEmpty() )
    element.setAttribute( "encodingStyle", mEncodingStyle );
#endif
  if ( !mPart.isEmpty() )
    element.setAttribute( QLatin1String("part"), mPart );
  if ( !mNameSpace.isEmpty() )
    element.setAttribute( QLatin1String("namespace"), mNameSpace );

  if ( mUse == LiteralUse )
    element.setAttribute( QLatin1String("use"), QLatin1String("literal") );
  else
    element.setAttribute( QLatin1String("use"), QLatin1String("encoded" ));
}


SoapBinding::Fault::Fault()
  : mUse( LiteralUse )
{
}

SoapBinding::Fault::~Fault()
{
}

#if 0
void SoapBinding::Fault::setEncodingStyle( const QString &encodingStyle )
{
  mEncodingStyle = encodingStyle;
}

QString SoapBinding::Fault::encodingStyle() const
{
  return mEncodingStyle;
}
#endif

void SoapBinding::Fault::setUse( Use use )
{
  mUse = use;
}

SoapBinding::Use SoapBinding::Fault::use() const
{
  return mUse;
}

void SoapBinding::Fault::setNameSpace( const QString &nameSpace )
{
  mNameSpace = nameSpace;
}

QString SoapBinding::Fault::nameSpace() const
{
  return mNameSpace;
}
void SoapBinding::Fault::loadXML( ParserContext *context, const QDomElement &element )
{
#if 0
  if ( element.hasAttribute( "encodingStyle" ) )
    mEncodingStyle = element.attribute( "encodingStyle" );
  else
    mEncodingStyle = QString();
#endif

  mNameSpace = element.attribute( QLatin1String("namespace") );

  if ( element.hasAttribute( QLatin1String("use" )) ) {
    if ( element.attribute( QLatin1String("use") ) == QLatin1String("literal") )
      mUse = LiteralUse;
    else if ( element.attribute( QLatin1String("use") ) == QLatin1String("encoded") )
      mUse = EncodedUse;
    else
      context->messageHandler()->warning( QLatin1String("SoapBinding::Fault: unknown 'use' value") );
  }
}

void SoapBinding::Fault::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElementNS( soapStandardNamespace, soapPrefix( context ) + QLatin1String("fault") );
  parent.appendChild( element );

#if 0
  if ( !mEncodingStyle.isEmpty() )
    element.setAttribute( "encodingStyle", mEncodingStyle );
#endif
  if ( !mNameSpace.isEmpty() )
    element.setAttribute( QLatin1String("namespace"), mNameSpace );

  if ( mUse == LiteralUse )
    element.setAttribute( QLatin1String("use"), QLatin1String("literal") );
  else
    element.setAttribute( QLatin1String("use"), QLatin1String("encoded") );
}


SoapBinding::Header::Header()
  : mUse( LiteralUse )
{
}

SoapBinding::Header::~Header()
{
}

void SoapBinding::Header::setHeaderFault( const HeaderFault &headerFault )
{
  mHeaderFault = headerFault;
}

SoapBinding::HeaderFault SoapBinding::Header::headerFault() const
{
  return mHeaderFault;
}

void SoapBinding::Header::setMessage( const QName &message )
{
  mMessage = message;
}

QName SoapBinding::Header::message() const
{
  return mMessage;
}

void SoapBinding::Header::setPart( const QString &part )
{
  mPart = part;
}

QString SoapBinding::Header::part() const
{
  return mPart;
}

void SoapBinding::Header::setUse( const Use &use )
{
  mUse = use;
}

SoapBinding::Use SoapBinding::Header::use() const
{
  return mUse;
}

#if 0
void SoapBinding::Header::setEncodingStyle( const QString encodingStyle )
{
  mEncodingStyle = encodingStyle;
}

QString SoapBinding::Header::encodingStyle() const
{
  return mEncodingStyle;
}
#endif

void SoapBinding::Header::setNameSpace( const QString &nameSpace )
{
  mNameSpace = nameSpace;
}

QString SoapBinding::Header::nameSpace() const
{
  return mNameSpace;
}

void SoapBinding::Header::loadXML( ParserContext *context, const QDomElement &element )
{
  mMessage = element.attribute( QLatin1String("message") );
  if ( mMessage.isEmpty() )
    context->messageHandler()->warning( QLatin1String("SoapBinding::Header: 'message' required") );
  else {
    if ( !mMessage.prefix().isEmpty() )
      mMessage.setNameSpace( context->namespaceManager()->uri( mMessage.prefix() ) );
  }

  mPart = element.attribute( QLatin1String("part") );
  if ( mPart.isEmpty() )
    context->messageHandler()->warning( QLatin1String("SoapBinding::Header: 'part' required" ));

  if ( element.attribute( QLatin1String("use") ) == QLatin1String("literal") )
    mUse = LiteralUse;
  else if ( element.attribute(QLatin1String( "use") ) == QLatin1String("encoded") )
    mUse = EncodedUse;
  else if ( element.attribute( QLatin1String("use" )).isEmpty() )
    context->messageHandler()->warning( QLatin1String("SoapBinding::Header: 'use' required") );
  else
    context->messageHandler()->warning( QLatin1String("SoapBinding::Header: unknown 'use' value") );

#if 0
  if ( element.hasAttribute( "encodingStyle" ) )
    mEncodingStyle = element.attribute( "encodingStyle" );
  else
    mEncodingStyle = QString();
#endif

  mNameSpace = element.attribute( QLatin1String("namespace") );

  QDomElement child = element.firstChildElement();
  while ( !child.isNull() ) {
    NSManager namespaceManager( context, child );
    if ( child.tagName() == soapPrefix( context ) + QLatin1String("headerfault") ) {
      mHeaderFault.loadXML( context, child );
    } else {
      context->messageHandler()->warning( QString::fromLatin1( "SoapBinding::Header: unknown tag %1" ).arg( child.tagName() ) );
    }

    child = child.nextSiblingElement();
  }
}

void SoapBinding::Header::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElementNS( soapStandardNamespace, soapPrefix( context ) + QLatin1String("header") );
  parent.appendChild( element );

  if ( !mMessage.isEmpty() )
    element.setAttribute( QLatin1String("message"), mMessage.qname() );
  else
    context->messageHandler()->warning( QLatin1String("SoapBinding::Header: 'message' required") );

  if ( !mPart.isEmpty() )
    element.setAttribute( QLatin1String("part"), mPart );
  else
    context->messageHandler()->warning( QLatin1String("SoapBinding::Header: 'part' required") );

#if 0
  if ( !mEncodingStyle.isEmpty() )
    element.setAttribute( "encodingStyle", mEncodingStyle );
#endif
  if ( !mNameSpace.isEmpty() )
    element.setAttribute( QLatin1String("namespace"), mNameSpace );

  if ( mUse == LiteralUse )
    element.setAttribute( QLatin1String("use"), QLatin1String("literal") );
  else
    element.setAttribute( QLatin1String("use"), QLatin1String("encoded") );

  mHeaderFault.saveXML( context, document, element );
}


SoapBinding::HeaderFault::HeaderFault()
  : mUse( LiteralUse )
{
}

SoapBinding::HeaderFault::~HeaderFault()
{
}

void SoapBinding::HeaderFault::setMessage( const QName &message )
{
  mMessage = message;
}

QName SoapBinding::HeaderFault::message() const
{
  return mMessage;
}

#if 0
void SoapBinding::HeaderFault::setEncodingStyle( const QString &encodingStyle )
{
  mEncodingStyle = encodingStyle;
}

QString SoapBinding::HeaderFault::encodingStyle() const
{
  return mEncodingStyle;
}
#endif

void SoapBinding::HeaderFault::setPart( const QString &part )
{
  mPart = part;
}

QString SoapBinding::HeaderFault::part() const
{
  return mPart;
}

void SoapBinding::HeaderFault::setUse( Use use )
{
  mUse = use;
}

SoapBinding::Use SoapBinding::HeaderFault::use() const
{
  return mUse;
}

void SoapBinding::HeaderFault::setNameSpace( const QString &nameSpace )
{
  mNameSpace = nameSpace;
}

QString SoapBinding::HeaderFault::nameSpace() const
{
  return mNameSpace;
}

void SoapBinding::HeaderFault::loadXML( ParserContext *context, const QDomElement &element )
{
  mMessage = element.attribute( QLatin1String("message") );
  if ( mMessage.isEmpty() )
    context->messageHandler()->warning( QLatin1String("SoapBinding::HeaderFault: 'message' required") );

  mPart = element.attribute( QLatin1String("part") );
  if ( mPart.isEmpty() )
    context->messageHandler()->warning( QLatin1String("SoapBinding::HeaderFault: 'part' required") );

  if ( element.attribute( QLatin1String("use") ) == QLatin1String("literal") )
    mUse = LiteralUse;
  else if ( element.attribute(QLatin1String( "use") ) == QLatin1String("encoded") )
    mUse = EncodedUse;
  else if ( element.attribute( QLatin1String("use") ).isEmpty() )
    context->messageHandler()->warning( QLatin1String("SoapBinding::HeaderFault: 'use' required") );
  else
    context->messageHandler()->warning( QLatin1String("SoapBinding::HeaderFault: unknown 'use' value") );

#if 0
  if ( element.hasAttribute( "encodingStyle" ) )
    mEncodingStyle = element.attribute( "encodingStyle" );
  else
    mEncodingStyle = QString();
#endif

  mNameSpace = element.attribute( QLatin1String("namespace") );
}

void SoapBinding::HeaderFault::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElementNS( soapStandardNamespace, soapPrefix( context ) + QLatin1String("headerfault") );
  parent.appendChild( element );

  if ( !mMessage.isEmpty() )
    element.setAttribute( QLatin1String("message"), mMessage.qname() );
  else
    context->messageHandler()->warning( QLatin1String("SoapBinding::HeaderFault: 'message' required") );

  if ( !mPart.isEmpty() )
    element.setAttribute( QLatin1String("part"), mPart );
  else
    context->messageHandler()->warning( QLatin1String("SoapBinding::HeaderFault: 'part' required") );

#if 0
  if ( !mEncodingStyle.isEmpty() )
    element.setAttribute( "encodingStyle", mEncodingStyle );
#endif
  if ( !mNameSpace.isEmpty() )
    element.setAttribute( QLatin1String("namespace"), mNameSpace );

  if ( mUse == LiteralUse )
    element.setAttribute( QLatin1String("use"), QLatin1String("literal") );
  else
    element.setAttribute( QLatin1String("use"), QLatin1String("encoded") );
}


SoapBinding::Address::Address()
{
}

SoapBinding::Address::~Address()
{
}

void SoapBinding::Address::setLocation( const QUrl &location )
{
  mLocation = location;
}

QUrl SoapBinding::Address::location() const
{
  return mLocation;
}

void SoapBinding::setOperations( const Operation::Map &operations )
{
  mOperations = operations;
}

SoapBinding::Operation::Map SoapBinding::operations() const
{
  return mOperations;
}

void SoapBinding::Address::loadXML( ParserContext *context, const QDomElement &element )
{
  mLocation = element.attribute( QLatin1String("location") );
  if ( !mLocation.isValid() )
    context->messageHandler()->warning( QLatin1String("SoapBinding::Address: 'location' required") );
}

void SoapBinding::Address::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElementNS( soapStandardNamespace, soapPrefix( context ) + QLatin1String("address" ));
  parent.appendChild( element );

  if ( mLocation.isValid() )
    element.setAttribute( QLatin1String("location"), mLocation.toString() );
  else
    context->messageHandler()->warning( QLatin1String("SoapBinding::Address: 'location' required") );
}


SoapBinding::SoapBinding()
{
}

SoapBinding::~SoapBinding()
{
}

void SoapBinding::setAddress( const Address &address )
{
  mAddress = address;
}

SoapBinding::Address SoapBinding::address() const
{
  return mAddress;
}

void SoapBinding::setBinding( const Binding &binding )
{
  mBinding = binding;
}

SoapBinding::Binding SoapBinding::binding() const
{
  return mBinding;
}

void SoapBinding::parseBinding( ParserContext *context, const QDomElement &parent )
{
  mBinding.loadXML( context, parent );
}

void SoapBinding::parseOperation( ParserContext *context, const QString &name, const QDomElement &parent )
{
    QDomElement child = parent.firstChildElement();
    while ( !child.isNull() ) {
        NSManager namespaceManager( context, child );
        if ( NSManager::soapNamespaces().contains( namespaceManager.nameSpace(child) ) ) {
            if ( namespaceManager.localName(child) == QLatin1String("operation") ) {
                Operation op( name );
                op.loadXML( context, child );
                mOperations.insert( name, op );
            }
        }

        child = child.nextSiblingElement();
    }
}

// Parse <operation><input>
void SoapBinding::parseOperationInput( ParserContext *context, const QString &name, const QDomElement &parent )
{
    QDomElement child = parent.firstChildElement();
    while ( !child.isNull() ) {
        NSManager namespaceManager( context, child );
        //qDebug() << Q_FUNC_INFO << namespaceManager.localName(child) << namespaceManager.nameSpace(child);
        if ( NSManager::soapNamespaces().contains( namespaceManager.nameSpace(child) ) ) {
            if ( namespaceManager.localName(child) == QLatin1String("body") ) {
                Operation &op = mOperations[ name ];
                Body inputBody;
                inputBody.loadXML( context, child );
                op.setInput( inputBody );
            } else if ( namespaceManager.localName(child) == QLatin1String("header") ) {
                Operation &op = mOperations[ name ];
                Header inputHeader;
                inputHeader.loadXML( context, child );
                op.addInputHeader( inputHeader );
            }
        }
        child = child.nextSiblingElement();
    }
}

// Parse <operation><output>
void SoapBinding::parseOperationOutput( ParserContext *context, const QString &name, const QDomElement &parent )
{
    QDomElement child = parent.firstChildElement();
    while ( !child.isNull() ) {
        NSManager namespaceManager( context, child );
        if ( NSManager::soapNamespaces().contains( namespaceManager.nameSpace(child) ) ) {
            if ( namespaceManager.localName(child) == QLatin1String("body") ) {
                Operation &op = mOperations[ name ];
                Body outputBody;
                outputBody.loadXML( context, child );
                op.setOutput( outputBody );
            } else if ( namespaceManager.localName(child) == QLatin1String("header") ) {
                Operation &op = mOperations[ name ];
                Header outputHeader;
                outputHeader.loadXML( context, child );
                op.addOutputHeader( outputHeader );
            }
        }

        child = child.nextSiblingElement();
    }
}

void SoapBinding::parseOperationFault( ParserContext *context, const QString &name, const QDomElement &parent )
{
    QDomElement child = parent.firstChildElement();
    while ( !child.isNull() ) {
        NSManager namespaceManager( context, child );
        if ( NSManager::soapNamespaces().contains( namespaceManager.nameSpace(child) ) ) {
            if ( namespaceManager.localName(child) == QLatin1String("fault") ) {
                Operation &op = mOperations[ name ];
                Fault fault;
                fault.loadXML( context, child );
                op.setFault( fault );
            }
        }
        child = child.nextSiblingElement();
    }
}

void SoapBinding::parsePort( ParserContext *context, const QDomElement &parent )
{
    QDomElement child = parent.firstChildElement();
    while ( !child.isNull() ) {
        NSManager namespaceManager( context, child );
        if ( NSManager::soapNamespaces().contains( namespaceManager.nameSpace(child) ) ) {
            if ( namespaceManager.localName(child) == QLatin1String("address") ) {
                mAddress.loadXML( context, child );
            }
        }

        child = child.nextSiblingElement();
    }
}

void SoapBinding::synthesizeBinding( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  mBinding.saveXML( context, document, parent );
}

void SoapBinding::synthesizeOperation( ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent ) const
{
  const Operation &op = mOperations[ name ];
  op.saveXML( context, document, parent );
}

void SoapBinding::synthesizeOperationInput( ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent ) const
{
  const Operation &op = mOperations[ name ];
  op.input().saveXML( context, document, parent );
  Q_FOREACH(const Header& header, op.inputHeaders()) {
      header.saveXML( context, document, parent );
  }
}

void SoapBinding::synthesizeOperationOutput( ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent ) const
{
  const Operation &op = mOperations[ name ];
  op.output().saveXML( context, document, parent );
  Q_FOREACH(const Header& header, op.outputHeaders()) {
      header.saveXML( context, document, parent );
  }
}

void SoapBinding::synthesizeOperationFault( ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent ) const
{
  const Operation &op = mOperations[ name ];
  op.fault().saveXML( context, document, parent );
}

void SoapBinding::synthesizePort( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  mAddress.saveXML( context, document, parent );
}

bool SoapBinding::Headers::contains(const Header &other) const
{
    Q_FOREACH(const Header& header, *this) {
        if (header.mMessage == other.mMessage &&
            header.mPart == other.mPart &&
            header.mUse == other.mUse &&
#if 0
            header.mEncodingStyle == other.mEncodingStyle &&
#endif
            header.mNameSpace == other.mNameSpace)
            return true;
    }
    return false;
}
