/*
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2010 David Faure <dfaure@kdab.com>

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
#include <QDebug>

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

  // Keep the prefixes from the wsdl parsing, they are more meaningful than ns1 :)
  mNSManager = wsdl.namespaceManager();

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

#ifdef KDAB_TEMP
  mClasses.append( mSerializer );
#endif

//  mNSManager.dump();

  // TODO: allow server service
  convertClientService();
}

void Converter::convertTypes()
{
  // This is only used in "Document" style, to create the classes representing the types defined in the XSD.
  // It is not needed/used in "rpc" style.

  const XSD::Types types = mWSDL.definitions().type().types();

  XSD::ComplexType::List complexTypes = types.complexTypes();
  qDebug() << "Converting" << complexTypes.count() << "complex types";
  for ( int i = 0; i < complexTypes.count(); ++i )
    convertComplexType( &(complexTypes[ i ]) );

  XSD::SimpleType::List simpleTypes = types.simpleTypes();
  qDebug() << "Converting" << simpleTypes.count() << "simple types";
  for ( int i = 0; i < simpleTypes.count(); ++i )
    convertSimpleType( &(simpleTypes[ i ]) );

  XSD::Attribute::List attributes = types.attributes();
  qDebug() << "Converting" << attributes.count() << "attributes";
  for ( int i = 0; i < attributes.count(); ++i )
    convertAttribute( &(attributes[ i ]) );

  XSD::Element::List elements = types.elements();
  qDebug() << "Converting" << elements.count() << "elements";
  for ( int i = 0; i < elements.count(); ++i )
    convertElement( &(elements[ i ]) );
}
