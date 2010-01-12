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

using namespace KWSDL;

void Converter::createUtils()
{
#ifdef KDAB_DELETED
  mSerializer = KODE::Class( "Serializer" );
  mSerializer.addHeaderInclude( "QDomDocument" );
  mSerializer.addHeaderInclude( "QDateTime" );
  mSerializer.addHeaderInclude( "QString" );

  typedef struct {
    QString type;
    QString xsdType;
    QString marshalCode;
    QString demarshalCode;
  } TypeEntry;

  /**
    I know the following code looks a bit ugly, but it saves us a lot
    of typing and is easier to maintain at the end ;)
   */
  TypeEntry types[] = {
    { "QString", "string", "*value", "str" },
    { "bool", "boolean", "(*value ? \"true\" : \"false\")", "(str.toLower() == \"true\" ? true : false)" },
    { "float", "float", "QString::number( *value )", "str.toFloat()" },
    { "int", "int", "QString::number( *value )", "str.toInt()" },
    { "int", "integer", "QString::number( *value )", "str.toInt()" },
    { "unsigned int", "unsignedByte", "QString::number( *value )", "str.toUInt()" },
    { "unsigned int", "positiveInteger", "QString::number( *value )", "str.toUInt()" },
    { "double", "double", "QString::number( *value )", "str.toDouble()" },
    { "char", "byte", "QString( QChar( *value ) )", "str[ 0 ].toLatin1()" },
    { "unsigned char", "unsignedByte", "QString( QChar( *value ) )", "str[ 0 ].toLatin1()" },
    { "short", "short", "QString::number( *value )", "str.toShort()" },
    { "QByteArray", "base64Binary", "QString::fromUtf8( value->toBase64() )", "QByteArray::fromBase64( str.toUtf8() )" },
    { "QDateTime", "dateTime", "value->toString( Qt::ISODate )", "QDateTime::fromString( str, Qt::ISODate )" },
    { "QDate", "date", "value->toString( Qt::ISODate )", "QDate::fromString( str, Qt::ISODate )" },
    { "QTime", "time", "value->toString( Qt::ISODate )", "QTime::fromString( str, Qt::ISODate )" }
  };

  for ( uint i = 0; i < sizeof( types ) / sizeof( TypeEntry ); ++i ) {
    KODE::Function marshal, demarshal;
    KODE::Code code;

    TypeEntry entry = types[ i ];

    marshal = KODE::Function( "marshalValue", "QString" );
    marshal.setStatic( true );
    marshal.addArgument( "const " + entry.type + " *value" );

    code.clear();
    code += "if ( !value )";
    code.indent();
    code += "return QString();";
    code.unindent();
    code.newLine();
    code += "return " + entry.marshalCode + ';';
    marshal.setBody( code );

    mSerializer.addFunction( marshal );

    demarshal = KODE::Function( "demarshalValue", "void" );
    demarshal.setStatic( true );
    demarshal.addArgument( "const QString &str" );
    demarshal.addArgument( entry.type + " *value" );

    code.clear();
    code += "if ( !value )";
    code.indent();
    code += "return;";
    code.unindent();
    code.newLine();
    code += "*value = " + entry.demarshalCode + ';';
    demarshal.setBody( code );

    mSerializer.addFunction( demarshal );

    marshal = KODE::Function( "marshal", "void" );
    marshal.setStatic( true );
    marshal.addArgument( "QDomDocument &doc" );
    marshal.addArgument( "QDomElement &parent" );
    marshal.addArgument( "const QString &name" );
    marshal.addArgument( "const " + entry.type + " *value" );
    marshal.addArgument( "bool noNamespace" );

    code.clear();
    code += "if ( !value )";
    code.indent();
    code += "return;";
    code.unindent();
    code.newLine();
    code += "QDomElement parentElement;";
    code += "if ( !name.isEmpty() ) {";
    code.indent();
    code += "QDomElement element = doc.createElement( noNamespace ? name : \"" + mNSManager.schemaPrefix() + ":\" + name );";
    code += "element.setAttribute( \"" + mNSManager.schemaInstancePrefix() + ":type\", \"" + mNSManager.schemaPrefix() + ":" + entry.xsdType + "\" );";
    code += "parent.appendChild( element );";
    code += "parentElement = element;";
    code.unindent();
    code += "} else {";
    code.indent();
    code += "parentElement = parent;";
    code.unindent();
    code += '}';
    code.newLine();
    code += "parentElement.appendChild( doc.createTextNode( Serializer::marshalValue( value ) ) );";
    marshal.setBody( code );

    mSerializer.addFunction( marshal );

    demarshal = KODE::Function( "demarshal", "void" );
    demarshal.setStatic( true );
    demarshal.addArgument( "const QDomElement &element" );
    demarshal.addArgument( entry.type + " *value" );
    code.clear();
    code += "if ( !value )";
    code.indent();
    code += "return;";
    code.unindent();
    code.newLine();
    code += "Serializer::demarshalValue( element.text(), value );";
    demarshal.setBody( code );

    mSerializer.addFunction( demarshal );
  }
#endif
}

void Converter::createSoapUtils()
{
#ifdef KDAB_DELETED
  KODE::Class soapFault( "SoapFault" );
  soapFault.setDocs(
      "This class encapsulates error information.\n"
      "Whenever an error occur during transmission or the server reports "
      "an invalid request, a SoapFault object is returned which provides "
      "the following information:\n"
      "@li Code - A standard or custom error code.\n"
      "@li Description - An i18n'ed description of the error code.\n"
      "@li Actor - The actor which caused the error.\n"
      "@li Detail - Additional textual details about the error (e.g. backtraces).\n"
      "\n"
      "The Code and Description fields are mandatory."
      );

  KODE::Function ctor( "SoapFault" );
  ctor.setDocs( "Constructs an empty SoapFault object." );
  KODE::Function dtor( "~SoapFault" );
  dtor.setDocs( "Destroys the SoapFault object." );

  soapFault.addFunction( ctor );
  soapFault.addFunction( dtor );

  // variable definitions
  KODE::MemberVariable codeVar( "code", "QString" );
  soapFault.addMemberVariable( codeVar );

  KODE::MemberVariable descriptionVar( "description", "QString" );
  soapFault.addMemberVariable( descriptionVar );

  KODE::MemberVariable actorVar( "actor", "QString" );
  soapFault.addMemberVariable( actorVar );

  KODE::MemberVariable detailVar( "detail", "QString" );
  soapFault.addMemberVariable( detailVar );

  // setter methods
  KODE::Function setCode( "setCode", "void" );
  setCode.addArgument( "const QString &code" );
  setCode.setBody( codeVar.name() + " = code;" );
  setCode.setDocs( "Sets the code of this fault object." );

  KODE::Function setDescription( "setDescription", "void" );
  setDescription.addArgument( "const QString &description" );
  setDescription.setBody( descriptionVar.name() + " = description;" );
  setDescription.setDocs( "Sets the i18n'ed description of this fault object." );

  KODE::Function setActor( "setActor", "void" );
  setActor.addArgument( "const QString &actor" );
  setActor.setBody( actorVar.name() + " = actor;" );
  setActor.setDocs( "Sets the actor of this fault object." );

  KODE::Function setDetail( "setDetail", "void" );
  setDetail.addArgument( "const QString &detail" );
  setDetail.setBody( detailVar.name() + " = detail;" );
  setDetail.setDocs( "Sets additional details of this fault object." );

  soapFault.addFunction( setCode );
  soapFault.addFunction( setDescription );
  soapFault.addFunction( setActor );
  soapFault.addFunction( setDetail );

  // getter methods
  KODE::Function getCode( "code", "QString" );
  getCode.setConst( true );
  getCode.setBody( "return " + codeVar.name() + ';' );
  getCode.setDocs( "Returns the code of this fault object." );

  KODE::Function getDescription( "description", "QString" );
  getDescription.setConst( true );
  getDescription.setBody( "return " + descriptionVar.name() + ';' );
  getDescription.setDocs( "Returns the description of this fault object." );

  KODE::Function getActor( "actor", "QString" );
  getActor.setConst( true );
  getActor.setBody( "return " + actorVar.name() + ';' );
  getActor.setDocs( "Returns the actor of this fault object." );

  KODE::Function getDetail( "detail", "QString" );
  getDetail.setConst( true );
  getDetail.setBody( "return " + detailVar.name() + ';' );
  getDetail.setDocs( "Returns additional details of this fault object." );

  soapFault.addFunction( getCode );
  soapFault.addFunction( getDescription );
  soapFault.addFunction( getActor );
  soapFault.addFunction( getDetail );

  mClasses.append( soapFault );

  // serializer
  KODE::Function marshal( "marshal", "void" );
  marshal.setStatic( true );
  marshal.addArgument( "QDomDocument &doc" );
  marshal.addArgument( "QDomElement &parent" );
  marshal.addArgument( "const QString &name" );
  marshal.addArgument( "const SoapFault *value" );

  KODE::Code code;
  code.clear();
  code += "if ( !value )";
  code.indent();
  code += "return;";
  code.unindent();
  code.newLine();
  code += "QDomElement fault = doc.createElement( \"Fault\" );";
  code += "parent.appendChild( fault );";
  code.newLine();
  code += "if ( !value->code().isEmpty() ) {";
  code.indent();
  code += "QDomElement element = doc.createElement( \"faultcode\" );";
  code += "element.appendChild( doc.createTextNode( value->code() ) );";
  code += "fault.appendChild( element );";
  code.unindent();
  code += '}';
  code.newLine();
  code += "if ( !value->description().isEmpty() ) {";
  code.indent();
  code += "QDomElement element = doc.createElement( \"faultstring\" );";
  code += "element.appendChild( doc.createTextNode( value->description() ) );";
  code += "fault.appendChild( element );";
  code.unindent();
  code += '}';
  code.newLine();
  code += "if ( !value->actor().isEmpty() ) {";
  code.indent();
  code += "QDomElement element = doc.createElement( \"faultactor\" );";
  code += "element.appendChild( doc.createTextNode( value->actor() ) );";
  code += "fault.appendChild( element );";
  code.unindent();
  code += '}';
  code.newLine();
  code += "if ( !value->detail().isEmpty() ) {";
  code.indent();
  code += "QDomElement element = doc.createElement( \"detail\" );";
  code += "element.appendChild( doc.createTextNode( value->detail() ) );";
  code += "fault.appendChild( element );";
  code.unindent();
  code += '}';
  code.newLine();
  marshal.setBody( code );

  mSerializer.addFunction( marshal );

  KODE::Function demarshal( "demarshal", "void" );
  demarshal.setStatic( true );
  demarshal.addArgument( "const QDomElement &element" );
  demarshal.addArgument( "SoapFault *value" );
  code.clear();
  code += "if ( !value )";
  code.indent();
  code += "return;";
  code.unindent();
  code.newLine();
  code += "QDomNode node = element.firstChild();";
  code += "while ( !node.isNull() ) {";
  code.indent();
  code += "QDomElement child = node.toElement();";
  code += "if ( !child.isNull() ) {";
  code.indent();
  code += "if ( child.tagName() == \"faultcode\" )";
  code.indent();
  code += "value->setCode( child.text() );";
  code.unindent();
  code.newLine();
  code += "if ( child.tagName() == \"faultstring\" )";
  code.indent();
  code += "value->setDescription( child.text() );";
  code.unindent();
  code.newLine();
  code += "if ( child.tagName() == \"faultactor\" )";
  code.indent();
  code += "value->setActor( child.text() );";
  code.unindent();
  code.newLine();
  code += "if ( child.tagName() == \"detail\" )";
  code.indent();
  code += "value->setDetail( child.text() );";
  code.unindent();
  code.newLine();
  code.unindent();
  code += '}';
  code += "node = node.nextSibling();";
  code.unindent();
  code += '}';
  demarshal.setBody( code );

  mSerializer.addFunction( demarshal );
#endif
}
