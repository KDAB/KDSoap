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
