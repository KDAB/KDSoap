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
  mService.setNameSpace( mTargetNamespace );
}

QString Definitions::targetNamespace() const
{
  return mTargetNamespace;
}

void Definitions::setBindings( const Binding::List &bindings )
{
  mBindings = bindings;
}

Binding::List Definitions::bindings() const
{
  return mBindings;
}

void Definitions::setImports( const Import::List &imports )
{
  mImports = imports;
}

Import::List Definitions::imports() const
{
  return mImports;
}

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

void Definitions::setService( const Service &service )
{
  mService = service;
}

Service Definitions::service() const
{
  return mService;
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
  setTargetNamespace( element.attribute( "targetNamespace" ) );
  mName = element.attribute( "name" );

  QDomNamedNodeMap attributes = element.attributes();
  for ( int i = 0; i < attributes.count(); ++i ) {
    QDomAttr attribute = attributes.item( i ).toAttr();
    if ( attribute.name().startsWith( "xmlns:" ) ) {
      QString prefix = attribute.name().mid( 6 );
      //qDebug() << "Setting prefix" << prefix << "for ns" << attribute.value();
      context->namespaceManager()->setPrefix( prefix, attribute.value() );
    }
  }

  bool foundService = false;
  QDomElement child = element.firstChildElement();
  while ( !child.isNull() ) {
    QName tagName = child.tagName();
    if ( tagName.localName() == "import" ) {
      Import import( mTargetNamespace );
      import.loadXML( context, child );
      mImports.append( import );
    } else if ( tagName.localName() == "types" ) {
      mType.loadXML( context, child );
    } else if ( tagName.localName() == "message" ) {
      Message message( mTargetNamespace );
      message.loadXML( context, child );
      //qDebug() << "Definitions: found message" << message.name() << message.nameSpace();
      mMessages.append( message );
    } else if ( tagName.localName() == "portType" ) {
      PortType portType( mTargetNamespace );
      portType.loadXML( context, child );
      mPortTypes.append( portType );
    } else if ( tagName.localName() == "binding" ) {
      Binding binding( mTargetNamespace );
      binding.loadXML( context, child );
      mBindings.append( binding );
    } else if ( tagName.localName() == "service" ) {
      const QString name = child.attribute( "name" );
      qDebug() << "Service:" << name << "looking for" << mWantedService;
      // is this the service we want?
      if ( mWantedService.isEmpty() || mWantedService == name ) {
        if ( !foundService ) {
          mService.loadXML( context, &mBindings, child );
          foundService = true;
        } else {
          qDebug() << "WARNING: multiple service tags found. Use -s to specify the one you want.";
        }
      }
    } else if ( tagName.localName() == "documentation" ) {
      // ignore documentation for now
    } else {
      context->messageHandler()->warning( QString( "Definitions: unknown tag %1" ).arg( child.tagName() ) );
    }

    child = child.nextSiblingElement();
  }

  if ( !foundService ) {
    qDebug() << "WARNING: no service tags found. There is nothing to generate!";
    return false;
  }
  return true;
}

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

void Definitions::setWantedService(const QString &name)
{
  mWantedService = name;
}
