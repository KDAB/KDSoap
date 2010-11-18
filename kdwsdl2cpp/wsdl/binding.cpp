/****************************************************************************
** Copyright (C) 2010-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>
#include <QDebug>

#include "binding.h"

using namespace KWSDL;

static QString soapStandardNamespace = "http://schemas.xmlsoap.org/wsdl/soap/";
static QString httpStandardNamespace = "http://schemas.xmlsoap.org/wsdl/http/";

Binding::Binding()
  : mType( UnknownBinding )
{
}

Binding::Binding( const QString &nameSpace )
  : Element( nameSpace ), mType( UnknownBinding )
{
}

Binding::~Binding()
{
}

void Binding::loadXML( ParserContext *context, const QDomElement &element )
{
  mName = element.attribute( "name" );
  if ( mName.isEmpty() )
    context->messageHandler()->warning( "Binding: 'name' required" );

  mPortTypeName = element.attribute( "type" );
  if ( mPortTypeName.isEmpty() )
    context->messageHandler()->warning( "Binding: 'type' required" );
  else
    if ( mPortTypeName.nameSpace().isEmpty() )
      mPortTypeName.setNameSpace( nameSpace() );

  QDomElement child = element.firstChildElement();
  while ( !child.isNull() ) {
    QName tagName = child.tagName();
    if ( tagName.localName() == "operation" ) {
      BindingOperation operation( nameSpace() );
      operation.loadXML( &mSoapBinding, context, child );
      mOperations.append( operation );
    } else if ( child.tagName() == context->namespaceManager()->fullName( soapStandardNamespace, "binding" ) ) {
      mType = SOAPBinding;
      mSoapBinding.parseBinding( context, child );
    } else if ( child.tagName() == context->namespaceManager()->fullName( httpStandardNamespace, "binding" ) ) {
      mType = HTTPBinding;
      // Not Implemented: HTTPBinding
    } else {
      // Not Implemented: MIMEBinding
    }

    child = child.nextSiblingElement();
  }
}

void Binding::saveXML( ParserContext *context, QDomDocument &document, QDomElement &parent ) const
{
  QDomElement element = document.createElement( "binding" );
  parent.appendChild( element );

  if ( !mName.isEmpty() )
    element.setAttribute( "name", mName );
  else
    context->messageHandler()->warning( "Binding: 'name' required" );

  if ( !mPortTypeName.isEmpty() )
    element.setAttribute( "type", mPortTypeName.localName() );
  else
    context->messageHandler()->warning( "Binding: 'type' required" );

  mSoapBinding.synthesizeBinding( context, document, element );

  BindingOperation::List::ConstIterator it( mOperations.begin() );
  const BindingOperation::List::ConstIterator endIt( mOperations.end() );
  for ( ; it != endIt; ++it )
    (*it).saveXML( &mSoapBinding, context, document, element );
}

void Binding::setName( const QString &name )
{
  mName = name;
}

QString Binding::name() const
{
  return mName;
}

void Binding::setPortTypeName( const QName &portTypeName )
{
  mPortTypeName = portTypeName;
}

QName Binding::portTypeName() const
{
  return mPortTypeName;
}

void Binding::setOperations( const BindingOperation::List &operations )
{
  mOperations = operations;
}

BindingOperation::List Binding::operations() const
{
  return mOperations;
}

void Binding::setType( Type type )
{
  mType = type;
}

Binding::Type Binding::type() const
{
  return mType;
}

#if 0
void Binding::setSoapBinding( const SoapBinding &soapBinding )
{
  mSoapBinding = soapBinding;
}
#endif

SoapBinding Binding::soapBinding() const
{
  return mSoapBinding;
}

const AbstractBinding *Binding::binding() const
{
  if ( mType == SOAPBinding )
    return &mSoapBinding;
  else // Not Implemented: HTTPBinding and MIMEBinding
    return 0;
}
