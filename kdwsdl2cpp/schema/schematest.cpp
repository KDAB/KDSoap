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

#include <QFile>
#include <QDebug>

#include <schema/parser.h>

#include <common/nsmanager.h>
#include <common/messagehandler.h>
#include <common/parsercontext.h>

int main( int argc, char **argv )
{
  if ( argc != 2 ) {
    qDebug( "Missing argument: filename of schema" );
    return 1;
  }

  QString filename = argv[ 1 ];

  QFile file( filename );

  if ( !file.open( QIODevice::ReadOnly ) ) {
    qDebug( "Can't open file %s", qPrintable( file.fileName() ) );
    return 1;
  }

  NSManager namespaceManager;
  MessageHandler messageHandler;
  ParserContext context;
  context.setNamespaceManager( &namespaceManager );
  context.setMessageHandler( &messageHandler );

  XSD::Parser parser;
  if ( !parser.parseFile( &context, file ) ) {
    qDebug() << "Error parsing file " << filename;
    return 1;
  }

  XSD::Types types = parser.types();

  const XSD::SimpleType::List simpleTypes = types.simpleTypes();
  for ( int i = 0; i < simpleTypes.count(); ++i ) {
    XSD::SimpleType t = simpleTypes[ i ];
    qDebug() << "SimpleType: " << t.name() << t.baseTypeName().qname()
      << t.subType();
    qDebug() << "FacetType: " << t.facetType();
    if ( t.facetType() == XSD::SimpleType::ENUM ) {
      qDebug() << "  ENUMS " << t.facetEnums();
    }
  }

  const XSD::ComplexType::List complexTypes = types.complexTypes();
  for ( int i = 0; i < complexTypes.count(); ++i ) {
    qDebug( "ComplexType: %s %s", qPrintable( complexTypes[ i ].name() ), qPrintable( complexTypes[ i ].baseTypeName().qname() ) );
    const XSD::Element::List elements = complexTypes[ i ].elements();
    for ( int j = 0; j < elements.count(); ++j ) {
      qDebug( "\tElement: %s %s", qPrintable( elements[ j ].name() ), qPrintable( elements[ j ].type().qname() ) );
    }
    const XSD::Attribute::List attributes = complexTypes[ i ].attributes();
    for ( int j = 0; j < attributes.count(); ++j ) {
      qDebug( "\tAttribute: %s %s", qPrintable( attributes[ j ].name() ), qPrintable( attributes[ j ].type().qname() ) );
    }
  }

  const XSD::Element::List elements = types.elements();
  for ( int i = 0; i < elements.count(); ++i ) {
    qDebug( "Element: %s %s", qPrintable( elements[ i ].name() ), qPrintable( elements[ i ].type().qname() ) );
    foreach( XSD::Annotation a,elements[ i ].annotations() ) {
      qDebug() << "  Annotation:" << a.domElement().tagName();
    }
  }

  const XSD::Attribute::List attributes = types.attributes();
  for ( int i = 0; i < attributes.count(); ++i ) {
    qDebug( "Attribute: %s %s", qPrintable( attributes[ i ].name() ), qPrintable( attributes[ i ].type().qname() ) );
  }
  
  const XSD::AttributeGroup::List attributeGroups = types.attributeGroups();
  for ( int i = 0; i < attributeGroups.count(); ++i ) {
    qDebug( "AttributeGroup: %s", qPrintable( attributeGroups[ i ].name() ) );
  }

  foreach( XSD::Annotation a, parser.annotations() ) {
    qDebug() << "Annotation:" << a.domElement().tagName();
  }

  return 0;
}
