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

#include "converter.h"
#include <QDebug>

using namespace KWSDL;

void Converter::convertElement( const XSD::Element *element )
{
  const QString className( mTypeMap.localTypeForElement( QName( element->nameSpace(), element->name() ) ) );
  KODE::Class newClass( className );

  //newClass.addInclude( QString(), "Serializer" );

  if ( mTypeMap.isBuiltinType( element->type() ) ) {
    QString typeName = mTypeMap.localType( element->type() );

    KODE::Code ctorCode;
    KODE::Code dtorCode;

    // member variables
    KODE::MemberVariable variable( "value", typeName + '*' );
    newClass.addMemberVariable( variable );

    ctorCode += variable.name() + " = 0;";
    dtorCode += "delete " + variable.name() + "; " + variable.name() + " = 0;";

    // setter method
    KODE::Function setter( "setValue", "void" );
    setter.addArgument( typeName + " *value" );
    KODE::Code setterBody;
    setterBody += variable.name() + " = value;";
    setter.setBody( setterBody );

    // getter method
    KODE::Function getter( "value", typeName + '*' );
    getter.setBody( "return " + variable.name() + ';' );
    getter.setConst( true );

    // convenience constructor
    KODE::Function conctor( upperlize( newClass.name() ) );
    conctor.addArgument( typeName + " *value" );
    KODE::Code code;
    code += variable.name() + " = value;";
    conctor.setBody( code );

    if ( typeName == "QString" ) {
      KODE::Function charctor( upperlize( newClass.name() ) );
      charctor.addArgument( "const char *charValue" );
      KODE::Code code;
      code += variable.name() + " = new QString( charValue );";
      charctor.setBody( code );

      newClass.addFunction( charctor );
    }

    // type operator
    KODE::Function op( "operator const " + typeName + '*' );
    op.setBody( "return " + variable.name() + ';' );
    op.setConst( true );

    KODE::Function ctor( className );
    ctor.setBody( ctorCode );
    KODE::Function dtor( '~' + className );
    dtor.setBody( dtorCode );

    newClass.addFunction( ctor );
    newClass.addFunction( dtor );
    newClass.addFunction( conctor );
    newClass.addFunction( op );
    newClass.addFunction( setter );
    newClass.addFunction( getter );
  } else {
    // we inherit from the anonymous type, so we can provide the same
    // interface as the anonymous type
    QString anonName = mTypeMap.localType( element->type() );
    if (!anonName.isEmpty()) {
      qDebug() << "converter_element: adding base class" << anonName << "for" << newClass.name();
      newClass.addBaseClass( KODE::Class( anonName ) );
    }
  }

  if ( !element->documentation().isEmpty() )
    newClass.setDocs( element->documentation().simplified() );

  mClasses.append( newClass );

  createElementSerializer( element );
}

void Converter::createElementSerializer( const XSD::Element *element )
{
  QString className( mTypeMap.localTypeForElement( QName( element->nameSpace(), element->name() ) ) );
  //qDebug() << "would create serializer for" << className << mTypeMap.forwardDeclarationsForElement( element->qualifiedName() );

#ifdef KDAB_DELETED
  // include header
  mSerializer.addIncludes( QStringList(), mTypeMap.forwardDeclarationsForElement( element->qualifiedName() ) );

  KODE::Function marshal( "marshal", "void" );
  marshal.setStatic( true );
  marshal.addArgument( "QDomDocument &doc" );
  marshal.addArgument( "QDomElement &parent" );
  marshal.addArgument( "const QString &name" );
  marshal.addArgument( "const " + className + " *value" );
  marshal.addArgument( "bool noNamespace" );

  KODE::Function demarshal( "demarshal", "void" );
  demarshal.setStatic( true );
  demarshal.addArgument( "const QDomElement &parent" );
  demarshal.addArgument( className + " *value" );

  KODE::Code marshalCode, demarshalCode, code;

  marshalCode += "if ( !value )";
  marshalCode.indent();
  marshalCode += "return;";
  marshalCode.unindent();
  marshalCode.newLine();

  demarshalCode += "if ( !value )";
  demarshalCode.indent();
  demarshalCode += "return;";
  demarshalCode.unindent();
  demarshalCode.newLine();

  QString typeName = mTypeMap.localType( element->type() );
  if ( mTypeMap.isBuildinType( element->type() ) ) {
    marshalCode += "QDomElement element = doc.createElement( name );";
    marshalCode += "parent.appendChild( element );";
    marshalCode.newLine();
    marshalCode += "element.appendChild( doc.createTextNode( Serializer::marshalValue( value->value() ) ) );";

    demarshalCode += "const QString text = parent.text();";
    demarshalCode.newLine();
    demarshalCode += "if ( !text.isEmpty() ) {";
    demarshalCode.indent();
    demarshalCode += typeName + " *data = new " + typeName + "();";
    demarshalCode += "Serializer::demarshalValue( text, data );";
    demarshalCode += "value->setValue( data );";
    demarshalCode.unindent();
    demarshalCode += '}';
  } else {
    marshalCode += "QDomElement element = doc.createElement( name );";
    marshalCode += "parent.appendChild( element );";
    marshalCode.newLine();
    marshalCode += "Serializer::marshal( doc, element, QString(), (" + typeName + "*)value, noNamespace );";

    demarshalCode += "Serializer::demarshal( parent, (" + typeName + "*)value );";
  }

  marshal.setBody( marshalCode );
  demarshal.setBody( demarshalCode );

  mSerializer.addFunction( marshal );
  mSerializer.addFunction( demarshal );
#endif
}
