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

#include "converter.h"

#include <QDebug>

using namespace KWSDL;

static QString escapeEnum( const QString& );
static KODE::Code createRangeCheckCode( const XSD::SimpleType*, const QString&, KODE::Class& );

// Overall logic:
// if ENUM -> define "Type" and "type" variable
// if restricts a basic type or another simple type -> "value" variable
// else if list -> define a QList

void Converter::convertSimpleType( const XSD::SimpleType *type, const XSD::SimpleType::List& simpleTypeList )
{
  const QString typeName( mTypeMap.localType( type->qualifiedName() ) );
  //qDebug() << "convertSimpleType:" << type->qualifiedName() << typeName;
  KODE::Class newClass( typeName );

  QString classDocumentation;

  switch( type->subType() ) {
  case XSD::SimpleType::TypeRestriction: {
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
                            " you can also pass the enum directly. Example:\n" +
                            "someMethod(" + newClass.name() + "::" + enums.first() + ").";

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
      classDocumentation = "This class encapsulates a simple type.\n";

      const QName baseName = type->baseTypeName();
      const QString baseTypeName = mTypeMap.localType( baseName );
      Q_ASSERT(!baseTypeName.isEmpty());

      QList<QName> parentBasicTypes;
      parentBasicTypes.append(baseName);
      QName currentType = baseName;
      Q_FOREVER {
          XSD::SimpleType::List::const_iterator it = simpleTypeList.findSimpleType( currentType );
          if ( it != simpleTypeList.constEnd() && (*it).isRestriction() ) {
              currentType = (*it).baseTypeName();
              parentBasicTypes.append( currentType );
              continue;
          }
          break;
      }

      classDocumentation += "Whenever you have to pass an object of type " + newClass.name() +
                            " you can also pass the value directly as a " + mTypeMap.localType( currentType ) + '.';
      // include header
      newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( baseName ) );
      newClass.addHeaderIncludes( mTypeMap.headerIncludes( baseName ) );

      // member variables
      KODE::MemberVariable variable( "value", baseTypeName );
      newClass.addMemberVariable( variable );

      // setter method
      KODE::Function setter( "setValue", "void" );
      setter.addArgument( mTypeMap.localInputType( baseName, QName() ) + " value" );
      KODE::Code setterBody;
      if ( type->facetType() != XSD::SimpleType::NONE ) {
        setterBody += createRangeCheckCode( type, "(value)", newClass );
        setterBody.newLine();
        setterBody += "if (!rangeOk)";
        setterBody.indent();
        setterBody += "qDebug( \"Invalid range in " + newClass.name() + "::" + setter.name() + "()\" );";
        setterBody.unindent();
        setterBody.newLine();
      }
      setterBody += variable.name() + " = value;"; // ### call setValue in base class?
      setter.setBody( setterBody );
      newClass.addFunction( setter );

      // getter method
      KODE::Function getter( "value", baseTypeName );
      getter.setBody( "return " + variable.name() + ';' );
      getter.setConst( true );
      newClass.addFunction( getter );

      // convenience constructor
      KODE::Function conctor( upperlize( newClass.name() ) );
      conctor.addArgument( mTypeMap.localInputType( baseName, QName() ) + " value" );
      conctor.addBodyLine( "setValue(value);" );
      newClass.addFunction( conctor );

      // even more convenient constructor, for the case of multiple-level simple-type restrictions
      //qDebug() << typeName << ": baseName=" << baseName << "further up:" << parentBasicTypes;
      if ( parentBasicTypes.count() > 1 ) {
          parentBasicTypes.removeLast(); // the top-most one is in "currentType", so it's the input arg.
          KODE::Function baseCtor( conctor.name() );
          baseCtor.addArgument( mTypeMap.localInputType( currentType, QName() ) + " value" );
          QString beginLine = "setValue(";
          QString endLine = ")";
          Q_FOREACH(const QName& base, parentBasicTypes) {
              beginLine += mTypeMap.localType( base ) + '(';
              endLine += ')';
          }
          baseCtor.addBodyLine( beginLine + "value" + endLine + ';' );
          newClass.addFunction( baseCtor );
      }

      // type operator
      KODE::Function op( "operator " + baseTypeName );
      op.setBody( "return " + variable.name() + ';' );
      op.setConst( true );
      newClass.addFunction( op );
    }
  }
  break;
  case XSD::SimpleType::TypeList: {
    classDocumentation = "This class encapsulates a list type.";

    newClass.addHeaderInclude( "QList" );
    const QName baseName = type->listTypeName();
    const QString itemTypeName = mTypeMap.localType( baseName );

    // include header
    newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( baseName ) );
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( baseName ) );

    // member variables
    KODE::MemberVariable variable( "entries", "QList<" + itemTypeName + ">" );
    newClass.addMemberVariable( variable );

    // setter method
    KODE::Function setter( "setEntries", "void" );
    setter.addArgument( "const QList<" + itemTypeName + ">& entries" );
    setter.setBody( variable.name() + " = entries;" );

    // getter method
    KODE::Function getter( "entries", "QList<" + itemTypeName + ">" );
    getter.setBody( "return " + variable.name() + ';' );
    getter.setConst( true );

    newClass.addFunction( setter );
    newClass.addFunction( getter );
  }
  break;
  case XSD::SimpleType::TypeUnion:
      classDocumentation = "This class encapsulates a union type. NOT IMPLEMENTED.";
      qDebug() << "ERROR: unions are not implemented";
      break;
  };

  if ( !type->documentation().isEmpty() )
    newClass.setDocs( type->documentation().simplified() );
  else
    newClass.setDocs( classDocumentation );

  createSimpleTypeSerializer( newClass, type, simpleTypeList );

  // Empty ctor. Needed for derived simpleTypes (which use this one as value).
  KODE::Function emptyCtor( upperlize( newClass.name() ) );
  newClass.addFunction( emptyCtor );

  // Empty dtor. Just in case ;)
  KODE::Function dtor( '~' + upperlize( newClass.name() ) );
  newClass.addFunction( dtor );

  mClasses.append( newClass );
}

void Converter::createSimpleTypeSerializer( KODE::Class& newClass, const XSD::SimpleType *type, const XSD::SimpleType::List& simpleTypeList )
{
    const QString typeName = mTypeMap.localType( type->qualifiedName() );

    KODE::Function serializeFunc( "serialize", "QVariant" );
    serializeFunc.setConst( true );

    KODE::Function deserializeFunc( "deserialize", "void" );
    deserializeFunc.addArgument( "const QVariant& value" );

    if ( type->subType() == XSD::SimpleType::TypeRestriction ) {
        // is an enumeration
        if ( type->facetType() & XSD::SimpleType::ENUM ) {
            const QStringList enums = type->facetEnums();
            QStringList escapedEnums;
            for ( int i = 0; i < enums.count(); ++i )
                escapedEnums.append( escapeEnum( enums[ i ] ) );

            KODE::MemberVariable variable( "type", "Type" ); // just for the naming

            {
                KODE::Code code;
                code += "switch ( " + variable.name() + " ) {";
                code.indent();
                for ( int i = 0; i < enums.count(); ++i ) {
                    code += "case " + typeName + "::" + escapedEnums[ i ] + ':';
                    code.indent();
                    code += "return QString::fromLatin1(\"" + enums[ i ] + "\");";
                    code.unindent();
                    /* add a hack for msvc because that one cannot parse switch statements
                       longer than a certain length, so start a new switch statement */
                    if(i % 64 == 63) {
                        code += "default:"; // silence gcc
                        code += "break;";
                        code.unindent();
                        code += '}';
                        code.newLine();
                        code += "switch ( " + variable.name() + " ) {";
                        code.indent();
                    }
                }
                code += "default:";
                code.indent();
                code += "qDebug(\"Unknown enum %d passed.\", " + variable.name() + ");";
                code += "break;";
                code.unindent();
                code.unindent();
                code += '}';
                code.newLine();
                code += "return QVariant();";
                serializeFunc.setBody( code );
            }
            {
                KODE::Code code;
                code += "static const struct { const char* name; Type value; } s_values[" + QString::number(enums.count()) + "] = {";
                for ( int i = 0; i < enums.count(); ++i ) {
                    code += "{ \"" + enums[ i ] + "\", " + typeName + "::" + escapedEnums[ i ] + " }" + (i < enums.count()-1 ? "," : "");
                }
                code += "};";
                code += "const QString str = value.toString();";
                code += "for ( int i = 0; i < " + QString::number(enums.count()) + "; ++i ) {";
                code.indent();
                code += "if (str == QLatin1String(s_values[i].name)) {";
                code.indent();
                code += variable.name() + " = s_values[i].value;";
                code += "return;";
                code.unindent();
                code += "}";
                code.unindent();
                code += "}";
                code += "qDebug(\"Unknown enum value '%s' passed to '" + newClass.name() + "'.\", qPrintable(str) );";
                deserializeFunc.setBody( code );
            }

        }
        if ( type->baseTypeName() != XmlAnyType
            && !type->baseTypeName().isEmpty()
            && !(type->facetType() & XSD::SimpleType::ENUM) ) {
            // 'inherits' a basic type or another simple type -> using value.

            const QName baseName = type->baseTypeName();
            const QString baseTypeName = mTypeMap.localType( baseName );
            Q_ASSERT(!baseTypeName.isEmpty());
            KODE::MemberVariable variable( "value", baseTypeName ); // just for the naming
            const QName baseType = type->baseTypeName();
            Q_UNUSED(simpleTypeList);
            //const QName mostBasicTypeName = simpleTypeList.mostBasicType( baseType );
            //Q_UNUSED(mostBasicTypeName);
            if ( mTypeMap.isBuiltinType( baseType ) ) { // serialize from QString, int, etc.
                serializeFunc.addBodyLine( "return QVariant::fromValue(" + variable.name() + ");" );
                deserializeFunc.addBodyLine( variable.name() + " = value.value<" + baseTypeName + ">();" );
            } else { // inherits another simple type, need to call its serialize/deserialize method
                serializeFunc.addBodyLine( "return " + variable.name() + ".serialize();" COMMENT );
                deserializeFunc.addBodyLine( variable.name() + ".deserialize( value );" COMMENT );
            }

        }
    } else {
        const QName baseName = type->listTypeName();
        const QString itemTypeName = mTypeMap.localType( baseName );
        KODE::MemberVariable variable( "entries", "QList<" + itemTypeName + ">" ); // just for the name
        {
            KODE::Code code;
            code += "QString str;";
            code += "for ( int i = 0; i < " + variable.name() + ".count(); ++i ) {";
            code.indent();
            code += "if (!str.isEmpty())";
            code.indent();
            code += "str += QLatin1Char(' ');";
            code.unindent();
            if ( itemTypeName == "QString") // special but common case, no conversion needed
                code += "str += " + variable.name() + ".at(i);";
            else if ( mTypeMap.isBuiltinType( baseName ) ) // serialize from int, float, bool, etc.
                code += "str += QVariant(" + variable.name() + ".at(i)).toString();";
            else
                code += "str += " + variable.name() + ".at(i).serialize().toString();";
            code.unindent();
            code += "}";
            code += "return str;";
            serializeFunc.setBody(code);
        }
        {
            newClass.addHeaderInclude("QtCore/QStringList");
            KODE::Code code;
            code += "const QStringList list = value.toString().split(QLatin1Char(' '));";
            code += "for (int i = 0; i < list.count(); ++i) {";
            code.indent();
            QString val = QString::fromLatin1("list.at(i)");
            if ( itemTypeName == "QString" )
                /*nothing to do*/;
            else if ( mTypeMap.isBuiltinType( baseName ) ) { // deserialize to int, float, bool, etc.
                val = "QVariant(" + val + ").value<" + itemTypeName + ">()";
            } else {
                code += itemTypeName + " tmp;";
                code += "tmp.deserialize(" + val + ");";
                val = "tmp";
            }
            code += variable.name() + ".append(" + val + ");";
            code.unindent();
            code += "}";
            deserializeFunc.setBody(code);
        }
    }

    newClass.addFunction( serializeFunc );
    newClass.addFunction( deserializeFunc );
}

static QString escapeEnum( const QString &str )
{
  QString enumStr = upperlize( str );
  enumStr.replace( "-", "_" );
  enumStr.replace( ":", "_" ); // xsd:int -> xsd_int  (testcase: salesforce-partner.wsdl)
  return enumStr;
}

static KODE::Code createRangeCheckCode( const XSD::SimpleType *type, const QString &variableName, KODE::Class &parentClass )
{
  KODE::Code code;
  code += "bool rangeOk = true;";
  code.newLine();

  // TODO range-check code for facetWhiteSpace, facetTotalDigits, facetFractionDigits
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
      code += "QRegExp exp( QString::fromLatin1(\"" + type->facetPattern() + "\") );";
    code += "rangeOk = rangeOk && exp.exactMatch( " + variableName + " );";

    parentClass.addInclude( "QRegExp" );
  }

  return code;
}
