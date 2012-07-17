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

#include "definitions.h"
#include <QDebug>

using namespace KWSDL;

Definitions::Definitions()
{
}

Definitions::~Definitions()
{
}

void Definitions::setName( const QString &name )
{
  mName = name;
}

QString Definitions::name() const
{
  return mName;
}

void Definitions::setTargetNamespace( const QString &targetNamespace )
{
  mTargetNamespace = targetNamespace;

  mType.setNameSpace( mTargetNamespace );
}

QString Definitions::targetNamespace() const
{
  return mTargetNamespace;
}

#if 0
void Definitions::setBindings( const Binding::List &bindings )
{
  mBindings = bindings;
}
#endif

Binding::List Definitions::bindings() const
{
  return mBindings;
}

#if 0
void Definitions::setImports( const Import::List &imports )
{
  mImports = imports;
}

Import::List Definitions::imports() const
{
  return mImports;
}
#endif

void Definitions::setMessages( const Message::List &messages )
{
  mMessages = messages;
}

Message::List Definitions::messages() const
{
  return mMessages;
}

void Definitions::setPortTypes( const PortType::List &portTypes )
{
  mPortTypes = portTypes;
}

PortType::List Definitions::portTypes() const
{
  return mPortTypes;
}

#if 0
void Definitions::setService( const Service &service )
{
  mService = service;
}
#endif

Service::List Definitions::services() const
{
  return mServices;
}

void Definitions::setType( const Type &type )
{
  mType = type;
}

Type Definitions::type() const
{
  return mType;
}

bool Definitions::loadXML( ParserContext *context, const QDomElement &element )
{
  setTargetNamespace( element.attribute( QLatin1String("targetNamespace") ) );
  mName = element.attribute( QLatin1String("name") );

  context->namespaceManager()->enterChild( element );

  QDomElement child = element.firstChildElement();
  while ( !child.isNull() ) {
    NSManager namespaceManager( context, child );
    const QName tagName( child.tagName() );
    if ( tagName.localName() == QLatin1String("import") ) {
      Import import( mTargetNamespace );
      import.loadXML( context, child );
      qFatal("Unsupported <import> element in <definitions> - TODO");
      //mImports.append( import );
    } else if ( tagName.localName() == QLatin1String("types") ) {
      if ( !mType.loadXML( context, child ) )
          return false;
    } else if ( tagName.localName() == QLatin1String("message") ) {
      Message message( mTargetNamespace );
      message.loadXML( context, child );
      //qDebug() << "Definitions: found message" << message.name() << message.nameSpace();
      mMessages.append( message );
    } else if ( tagName.localName() == QLatin1String("portType") ) {
      PortType portType( mTargetNamespace );
      portType.loadXML( context, child );
      mPortTypes.append( portType );
    } else if ( tagName.localName() == QLatin1String("binding") ) {
      Binding binding( mTargetNamespace );
      binding.loadXML( context, child );
      mBindings.append( binding );
    } else if ( tagName.localName() == QLatin1String("service") ) {
      const QString name = child.attribute( QLatin1String("name") );
      //qDebug() << "Service:" << name << "looking for" << mWantedService;
      // is this the service we want?
      if ( mWantedService.isEmpty() || mWantedService == name ) {
        Service service( mTargetNamespace );
        service.loadXML( context, &mBindings, child );
        mServices.append( service );
      }
    } else if ( tagName.localName() == QLatin1String("documentation") ) {
      // ignore documentation for now
    } else {
      context->messageHandler()->warning( QString::fromLatin1( "Definitions: unknown tag %1" ).arg( child.tagName() ) );
    }

    child = child.nextSiblingElement();
  }

  if ( mServices.isEmpty() ) {
    qDebug() << "WARNING: no service tags found. There is nothing to generate!";
    return false;
  }
  return true;
}

#if 0
void Definitions::saveXML( ParserContext *context, QDomDocument &document ) const
{
  QDomElement element = document.createElement( "definitions" );
  document.appendChild( element );

  if ( !mTargetNamespace.isEmpty() )
    element.setAttribute( "targetNamespace", mTargetNamespace );
  if ( !mName.isEmpty() )
    element.setAttribute( "name", mName );

  {
    Import::List::ConstIterator it( mImports.begin() );
    const Import::List::ConstIterator endIt( mImports.end() );
    for ( ; it != endIt; ++it )
      (*it).saveXML( context, document, element );
  }

  mType.saveXML( context, document, element );

  {
    Message::List::ConstIterator it( mMessages.begin() );
    const Message::List::ConstIterator endIt( mMessages.end() );
    for ( ; it != endIt; ++it )
      (*it).saveXML( context, document, element );
  }

  {
    PortType::List::ConstIterator it( mPortTypes.begin() );
    const PortType::List::ConstIterator endIt( mPortTypes.end() );
    for ( ; it != endIt; ++it )
      (*it).saveXML( context, document, element );
  }

  {
    Binding::List::ConstIterator it( mBindings.begin() );
    const Binding::List::ConstIterator endIt( mBindings.end() );
    for ( ; it != endIt; ++it )
      (*it).saveXML( context, document, element );
  }

  mService.saveXML( context, &mBindings, document, element );
}
#endif

void Definitions::setWantedService(const QString &name)
{
  mWantedService = name;
}
