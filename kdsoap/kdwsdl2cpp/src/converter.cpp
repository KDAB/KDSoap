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

#include "settings.h"

#include "converter.h"

using namespace KWSDL;

QString upperlize( const QString &str )
{
  return str[ 0 ].toUpper() + str.mid( 1 );
}

QString lowerlize( const QString &str )
{
  return str[ 0 ].toLower() + str.mid( 1 );
}


Converter::Converter()
{
  mQObject = KODE::Class( "QObject" );
}

void Converter::setWSDL( const WSDL &wsdl )
{
  mWSDL = wsdl;

  // merge namespaces from wsdl and schema
  QStringList namespaces = wsdl.definitions().type().types().namespaces();
  namespaces.append( "http://schemas.xmlsoap.org/soap/encoding/" );
  const QStringList wsdlNamespaces = wsdl.namespaceManager().uris();
  for ( int i = 0; i < wsdlNamespaces.count(); ++i ) {
    if ( !namespaces.contains( wsdlNamespaces[ i ] ) )
      namespaces.append( wsdlNamespaces[ i ] );
  }

  // create new prefix table
  for ( int i = 0; i < namespaces.count(); ++i )
    mNSManager.setPrefix( QString( "ns%1" ).arg( i + 1 ), namespaces[ i ] );

  // overwrite some default prefixes
  mNSManager.setPrefix( "soapenc", "http://schemas.xmlsoap.org/soap/encoding/" );
  mNSManager.setPrefix( "http", "http://schemas.xmlsoap.org/wsdl/http/" );
  mNSManager.setPrefix( "soap", "http://schemas.xmlsoap.org/wsdl/soap/" );
  mNSManager.setPrefix( "xsd", "http://www.w3.org/2001/XMLSchema" );
  mNSManager.setPrefix( "xsi", "http://www.w3.org/2001/XMLSchema-instance" );

  // overwrite with prefixes from settings
  Settings::NSMapping mapping = Settings::self()->namespaceMapping();
  Settings::NSMapping::Iterator it;
  for ( it = mapping.begin(); it != mapping.end(); ++it )
    mNSManager.setPrefix( it.value(), it.key() );

//  mNSManager.dump();

  mTypeMap.setNSManager( &mNSManager );

  // set the xsd types
  mTypeMap.addSchemaTypes( wsdl.definitions().type().types() );
//  mTypeMap.dump();
}

KODE::Class::List Converter::classes() const
{
  return mClasses;
}

void Converter::convert()
{
  createUtils();
  createSoapUtils();

  convertTypes();

  mClasses.append( mSerializer );

  // TODO: allow server service
  convertClientService();
}

void Converter::convertTypes()
{
  const XSD::Types types = mWSDL.definitions().type().types();

  XSD::ComplexType::List complexTypes = types.complexTypes();
  for ( int i = 0; i < complexTypes.count(); ++i )
    convertComplexType( &(complexTypes[ i ]) );

  XSD::SimpleType::List simpleTypes = types.simpleTypes();
  for ( int i = 0; i < simpleTypes.count(); ++i )
    convertSimpleType( &(simpleTypes[ i ]) );

  XSD::Attribute::List attributes = types.attributes();
  for ( int i = 0; i < attributes.count(); ++i )
    convertAttribute( &(attributes[ i ]) );

  XSD::Element::List elements = types.elements();
  for ( int i = 0; i < elements.count(); ++i )
    convertElement( &(elements[ i ]) );
}
