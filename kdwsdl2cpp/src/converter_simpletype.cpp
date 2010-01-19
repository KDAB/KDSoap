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

static QString escapeEnum( const QString& );
static KODE::Code createRangeCheckCode( const XSD::SimpleType*, const QString&, KODE::Class& );


void Converter::convertSimpleType( const XSD::SimpleType *type )
{
  const QString typeName( mTypeMap.localType( type->qualifiedName() ) );
  qDebug() << "convertSimpleType:" << type->qualifiedName().qname() << typeName;
  KODE::Class newClass( typeName );

  KODE::Code ctorBody;
  KODE::Code dtorBody;

  QString classDocumentation;

  if ( type->subType() == XSD::SimpleType::TypeRestriction ) {
    /**
      Use setter and getter method for enums as well.
     */
    if ( type->facetType() & XSD::SimpleType::ENUM ) {
      classDocumentation = "This class is a wrapper for an enumeration.\n";

      QStringList enums = type->facetEnums();
      for ( int i = 0; i < enums.count(); ++i )
        enums[ i ] = escapeEnum( enums[ i ] );

      newClass.addEnum( KODE::Enum( "Type", enums ) );

      classDocumentation += "Whenever you have to pass an object of type " + newClass.name() +
                            " you can also pass the enum directly. Example:\nsomeMethod( " + newClass.name() + "::" + enums.first() + " )).";

      // member variables
      KODE::MemberVariable variable( "type", "Type" );
      newClass.addMemberVariable( variable );

      // setter method
      KODE::Function setter( "setType", "void" );
      setter.addArgument( "Type type" );
      setter.setBody( variable.name() + " = type;" );

      // getter method
      KODE::Function getter( "type", upperlize( newClass.name() ) + "::Type" );
      getter.setBody( "return " + variable.name() + ';' );
      getter.setConst( true );

      // convenience constructor
      KODE::Function conctor( upperlize( newClass.name() ) );
      conctor.addArgument( "const Type &type" );
      KODE::Code code;
      code += variable.name() + " = type;";
      conctor.setBody( code );

      // type operator
      KODE::Function op( "operator Type" );
      op.setBody( "return " + variable.name() + ';' );
      op.setConst( true );

      newClass.addFunction( conctor );
      newClass.addFunction( setter );
      newClass.addFunction( getter );
      newClass.addFunction( op );
    }

    /**
      A class can't derive from basic types (e.g. int or unsigned char), so
      we add setter and getter methods to set the value of this class.
     */
    if ( type->baseTypeName() != XmlAnyType
        && !type->baseTypeName().isEmpty()
        && !(type->facetType() & XSD::SimpleType::ENUM) ) {
      classDocumentation = "This class encapsulates an basic type.\n";

      const QName baseName = type->baseTypeName();
      const QString typeName = mTypeMap.localType( baseName );
      Q_ASSERT(!typeName.isEmpty());

      classDocumentation += "Whenever you have to pass an object of type " + newClass.name() +
                            " you can also pass the value directly (e.g. someMethod( (" + typeName + ")value )).";
      // include header
      newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( baseName ) );
      newClass.addHeaderIncludes( mTypeMap.headerIncludes( baseName ) );

      // member variables
      KODE::MemberVariable variable( "value", typeName );
      newClass.addMemberVariable( variable );

      //ctorBody += variable.name() + " = 0;";
      //dtorBody += "delete " + variable.name() + "; " + variable.name() + " = 0;";

      // setter method
      KODE::Function setter( "setValue", "void" );
      setter.addArgument( mTypeMap.localInputType( baseName, QName() ) + " value" );
      KODE::Code setterBody;
      if ( type->facetType() != XSD::SimpleType::NONE ) {
        setterBody += createRangeCheckCode( type, "(value)", newClass );
        setterBody.newLine();
        setterBody += "if ( !rangeOk )";
        setterBody.indent();
        setterBody += "qDebug( \"Invalid range in " + newClass.name() + "::" + setter.name() + "()\" );";
        setterBody.unindent();
        setterBody.newLine();
      }
      setterBody += variable.name() + " = value;";
      setter.setBody( setterBody );

      // getter method
      KODE::Function getter( "value", typeName );
      getter.setBody( "return " + variable.name() + ';' );
      getter.setConst( true );

      // convenience constructor
      KODE::Function conctor( upperlize( newClass.name() ) );
      conctor.addArgument( mTypeMap.localInputType( baseName, QName() ) + " value" );
      KODE::Code code;
      code += "setValue( value );";
      conctor.setBody( code );

#if 0
      if ( typeName == "QString" ) {
        KODE::Function charctor( upperlize( newClass.name() ) );
        charctor.addArgument( "const char *charValue" );
        KODE::Code code;
        code += "QString value( charValue );";
        code += createRangeCheckCode( type, "(value)", newClass );
        code.newLine();
        code += "if ( !rangeOk )";
        code.indent();
        code += "qDebug( \"Invalid range in " + newClass.name() + "::" + charctor.name() + "()\" );";
        code.unindent();
        code.newLine();
        code += variable.name() + " = value;";
        charctor.setBody( code );

        newClass.addFunction( charctor );
      }
#endif

      // type operator
      KODE::Function op( "operator " + typeName );
      op.setBody( "return " + variable.name() + ';' );
      op.setConst( true );

      newClass.addFunction( conctor );
      newClass.addFunction( op );
      newClass.addFunction( setter );
      newClass.addFunction( getter );
    }
  } else if ( type->subType() == XSD::SimpleType::TypeList ) {
    classDocumentation = "This class encapsulates a list type.";

    newClass.addHeaderInclude( "QList" );
    const QName baseName = type->listTypeName();
    const QString typeName = mTypeMap.localType( baseName );

    // include header
    newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( baseName ) );
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( baseName ) );

    // member variables
    KODE::MemberVariable variable( "entries", "QList<" + typeName + ">" );
    newClass.addMemberVariable( variable );

    //ctorBody += variable.name() + " = 0;";
    //dtorBody += "qDeleteAll( *" + variable.name() + " );";
    //dtorBody += variable.name() + "->clear();";
    //dtorBody += "delete " + variable.name() + "; " + variable.name() + " = 0;";

    // setter method
    KODE::Function setter( "setEntries", "void" );
    setter.addArgument( "QList<" + typeName + "> entries" );
    setter.setBody( variable.name() + " = entries;" );

    // getter method
    KODE::Function getter( "entries", "QList<" + typeName + ">" );
    getter.setBody( "return " + variable.name() + ';' );
    getter.setConst( true );

    newClass.addFunction( setter );
    newClass.addFunction( getter );
  }

  if ( !type->documentation().isEmpty() )
    newClass.setDocs( type->documentation().simplified() );
  else
    newClass.setDocs( classDocumentation );

  createSimpleTypeSerializer( newClass, type );

  KODE::Function ctor( upperlize( newClass.name() ) );
  ctor.setBody( ctorBody );
  newClass.addFunction( ctor );

  KODE::Function dtor( '~' + upperlize( newClass.name() ) );
  dtor.setBody( dtorBody );
  newClass.addFunction( dtor );

  mClasses.append( newClass );
}

void Converter::createSimpleTypeSerializer( KODE::Class& newClass, const XSD::SimpleType *type )
{
    //const QString typeName = mTypeMap.localType( type->qualifiedName() );
    const QString baseType = mTypeMap.localType( type->baseTypeName() );

    KODE::Function serializeFunc( "serialize", baseType );
    serializeFunc.setConst( true );

    KODE::Function deserializeFunc( "deserialize", "void" );
    deserializeFunc.addArgument( mTypeMap.localInputType( type->baseTypeName(), QName() ) + " args" );

    serializeFunc.addBodyLine( "return " + baseType + "(); // TODO" );

    deserializeFunc.addBodyLine( "Q_UNUSED(args);/*TODO*/" );

    newClass.addFunction( serializeFunc );
    newClass.addFunction( deserializeFunc );

#ifdef KDAB_DELETED

  KODE::Function marshal( "marshal", "void" );
  marshal.setStatic( true );
  marshal.addArgument( "QDomDocument &doc" );
  marshal.addArgument( "QDomElement &parent" );
  marshal.addArgument( "const QString &name" );
  marshal.addArgument( "const " + typeName + " *value" );
  marshal.addArgument( "bool noNamespace" );

  KODE::Function demarshal( "demarshal", "void" );
  demarshal.setStatic( true );
  demarshal.addArgument( "const QDomElement &parent" );
  demarshal.addArgument( typeName + " *value" );

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

  // include header
  mSerializer.addIncludes( QStringList(), mTypeMap.forwardDeclarations( type->qualifiedName() ) );

  if ( type->subType() == XSD::SimpleType::TypeRestriction ) {
    // is an enumeration
    if ( type->facetType() & XSD::SimpleType::ENUM ) {
      QStringList enums = type->facetEnums();
      QStringList escapedEnums;
      for ( int i = 0; i < enums.count(); ++i )
        escapedEnums.append( escapeEnum( enums[ i ] ) );

      // marshal value
      KODE::Function marshalValue( "marshalValue", "QString" );
      marshalValue.setStatic( true );
      marshalValue.addArgument( "const " + typeName + " *value" );
      code += "if ( !value )";
      code.indent();
      code += "return QString();";
      code.unindent();
      code.newLine();
      code += "switch ( value->type() ) {";
      code.indent();
      for ( int i = 0; i < enums.count(); ++i ) {
        code += "case " + typeName + "::" + escapedEnums[ i ] + ':';
        code.indent();
        code += "return \"" + enums[ i ] + "\";";
        code += "break;";
        code.unindent();
      }
      code += "default:";
      code.indent();
      code += "qDebug( \"Unknown enum %d passed.\", value->type() );";
      code += "break;";
      code.unindent();
      code.unindent();
      code += '}';
      code.newLine();
      code += "return QString();";
      marshalValue.setBody( code );

      // marshal
      marshalCode += "QDomElement parentElement;";
      marshalCode += "if ( !name.isEmpty() ) {";
      marshalCode.indent();
      const QString typePrefix = mNSManager.prefix( type->qualifiedName().nameSpace() );
      marshalCode += "QDomElement root = doc.createElement( noNamespace ? name : \"" + typePrefix + ":\" + name );";
      marshalCode += "root.setAttribute( \"" + mNSManager.schemaInstancePrefix() + ":type\", \"" +
                     typePrefix + ':' + type->name() + "\" );";
      marshalCode += "parent.appendChild( root );";
      marshalCode += "parentElement = root;";
      marshalCode.unindent();
      marshalCode += "} else {";
      marshalCode.indent();
      marshalCode += "parentElement = parent;";
      marshalCode.unindent();
      marshalCode += '}';

      marshalCode += "parentElement.appendChild( doc.createTextNode( Serializer::marshalValue( value ) ) );";

      // demarshal value
      KODE::Function demarshalValue( "demarshalValue", "void" );
      demarshalValue.setStatic( true );
      demarshalValue.addArgument( "const QString &str" );
      demarshalValue.addArgument( typeName + " *value" );
      code.clear();
      code += "if ( !value )";
      code.indent();
      code += "return;";
      code.unindent();
      code.newLine();
      for ( int i = 0; i < enums.count(); ++i ) {
        code += "if ( str == \"" + enums[ i ] + "\" )";
        code.indent();
        code += "value->setType( " + typeName + "::" + escapedEnums[ i ] + " );";
        code.unindent();
        code.newLine();
      }
      demarshalValue.setBody( code );

      // demarshal
      demarshalCode += "Serializer::demarshalValue( parent.text(), value );";

      mSerializer.addFunction( marshalValue );
      mSerializer.addFunction( demarshalValue );
    } else if ( !type->baseTypeName().isEmpty() ) {
      marshalCode += "Serializer::marshal( doc, parent, name, value->value(), true );";

      demarshalCode += "const QString text = parent.text();";
      demarshalCode.newLine();
      demarshalCode += "if ( !text.isEmpty() ) {";
      demarshalCode.indent();
      demarshalCode += baseType + " *data = new " + baseType + "();";
      demarshalCode += "Serializer::demarshal( parent, value );";
      demarshalCode += "value->setValue( data );";
      demarshalCode.unindent();
      demarshalCode += '}';

      KODE::Function marshalValue( "marshalValue", "QString" );
      marshalValue.setStatic( true );
      marshalValue.addArgument( "const " + typeName + " *value" );
      marshalValue.setBody( "return Serializer::marshalValue( value->value() );" );

      mSerializer.addFunction( marshalValue );

      KODE::Function demarshalValue( "demarshalValue", "void" );
      demarshalValue.setStatic( true );
      demarshalValue.addArgument( "const QString &str" );
      demarshalValue.addArgument( typeName + " *value" );
      KODE::Code code;
      code += baseType + " *data = new " + baseType + "();";
      code += "Serializer::demarshalValue( str, data );";
      code += "value->setValue( data );";
      demarshalValue.setBody( code );

      mSerializer.addFunction( demarshalValue );
    }
  } else if ( type->subType() == XSD::SimpleType::TypeList ) {
    const QString listType = mTypeMap.localType( type->listTypeName() );

    mSerializer.addInclude( "QStringList" );

    marshalCode += "QStringList list;";
    marshalCode += "QList<" + listType + "*> *entries = value->entries();";
    marshalCode += "QList<" + listType + "*>::ConstIterator it;";
    marshalCode += "for ( it = entries->begin(); it != entries->end(); ++it ) {";
    marshalCode.indent();
    marshalCode += "list.append( Serializer::marshalValue( *it ) );";
    marshalCode += "++it;";
    marshalCode.unindent();
    marshalCode += '}';
    marshalCode.newLine();
    marshalCode += "QDomElement parentElement;";
    marshalCode += "if ( !name.isEmpty() ) {";
    marshalCode.indent();
    marshalCode += "QDomElement element = doc.createElement( noNamespace ? name : \"" + mNSManager.prefix( type->listTypeName().nameSpace() ) + ":\" + name );";
    marshalCode += "parent.appendChild( element );";
    marshalCode += "parentElement = element;";
    marshalCode.unindent();
    marshalCode += "} else {";
    marshalCode.indent();
    marshalCode += "parentElement = parent;";
    marshalCode.unindent();
    marshalCode += '}';
    marshalCode += "parentElement.appendChild( doc.createTextNode( list.join( \" \" ) ) );";

    demarshalCode += "const QStringList list = parent.text().split( \" \", QString::SkipEmptyParts );";
    demarshalCode += "if ( !list.isEmpty() ) {";
    demarshalCode.indent();
    demarshalCode += "QList<" + listType + "*> *entries = new QList<" + listType + "*>();";
    demarshalCode += "QStringList::ConstIterator it;";
    demarshalCode += "for ( it = list.begin(); it != list.end(); ++it ) {";
    demarshalCode.indent();
    demarshalCode += listType + " *entry = new " + listType + "();";
    demarshalCode += "Serializer::demarshalValue( *it, entry );";
    demarshalCode += "entries->append( entry );";
    demarshalCode.unindent();
    demarshalCode += '}';
    demarshalCode.newLine();
    demarshalCode += "value->setEntries( entries );";
    demarshalCode.unindent();
    demarshalCode += '}';
  }

  marshal.setBody( marshalCode );
  mSerializer.addFunction( marshal );

  demarshal.setBody( demarshalCode );
  mSerializer.addFunction( demarshal );
#endif
}

static QString escapeEnum( const QString &str )
{
  QString enumStr = upperlize( str );

  return enumStr.replace( "-", "_" );
}

static KODE::Code createRangeCheckCode( const XSD::SimpleType *type, const QString &variableName, KODE::Class &parentClass )
{
  KODE::Code code;
  code += "bool rangeOk = true;";
  code.newLine();

  // TODO
  /*
    WhiteSpaceType facetWhiteSpace() const;
    int facetTotalDigits() const;
    int facetFractionDigits() const;
  */

  if ( type->facetType() & XSD::SimpleType::MININC )
    code += "rangeOk = rangeOk && (" + variableName + " >= " + QString::number( type->facetMinimumInclusive() ) + ");";
  if ( type->facetType() & XSD::SimpleType::MINEX )
    code += "rangeOk = rangeOk && (" + variableName + " > " + QString::number( type->facetMinimumExclusive() ) + ");";
  if ( type->facetType() & XSD::SimpleType::MAXINC )
    code += "rangeOk = rangeOk && (" + variableName + " <= " + QString::number( type->facetMaximumInclusive() ) + ");";
  if ( type->facetType() & XSD::SimpleType::MINEX )
    code += "rangeOk = rangeOk && (" + variableName + " < " + QString::number( type->facetMaximumExclusive() ) + ");";

  if ( type->facetType() & XSD::SimpleType::LENGTH )
    code += "rangeOk = rangeOk && (" + variableName + ".length() == " + QString::number( type->facetLength() ) + ");";
  if ( type->facetType() & XSD::SimpleType::MINLEN )
    code += "rangeOk = rangeOk && (" + variableName + ".length() >= " + QString::number( type->facetMinimumLength() ) + ");";
  if ( type->facetType() & XSD::SimpleType::MAXLEN )
    code += "rangeOk = rangeOk && (" + variableName + ".length() <= " + QString::number( type->facetMaximumLength() ) + ");";
  if ( type->facetType() & XSD::SimpleType::PATTERN ) {
    code += "QRegExp exp( \"" + type->facetPattern() + "\" );";
    code += "rangeOk = rangeOk && exp.exactMatch( " + variableName + " );";

    parentClass.addInclude( "QRegExp" );
  }

  return code;
}
