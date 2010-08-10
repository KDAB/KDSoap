/*
    This file is part of KDE.

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

#include "converter.h"

#include <QDebug>

using namespace KWSDL;

static QString escapeEnum( const QString& );
static KODE::Code createRangeCheckCode( const XSD::SimpleType*, const QString&, KODE::Class& );

// Overall logic:
// if ENUM -> define "Type" and "type" variable
// if restricts a basic type or another simple type -> "value" variable
// else if list -> ...

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
    const QString typeName = mTypeMap.localType( baseName );

    // include header
    newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( baseName ) );
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( baseName ) );

    // member variables
    KODE::MemberVariable variable( "entries", "QList<" + typeName + ">" );
    newClass.addMemberVariable( variable );

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
                serializeFunc.addBodyLine( "return " + variable.name() + ".serialize();" );
                deserializeFunc.addBodyLine( variable.name() + ".deserialize( value );" );
            }

        }
    } else {
        // TODO lists
        serializeFunc.addBodyLine( "return QVariant(); // TODO (createSimpleTypeSerializer for lists)" );
        deserializeFunc.addBodyLine( "Q_UNUSED(value); // TODO (createSimpleTypeSerializer for lists)" );
    }

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
  enumStr.replace( "-", "_" );
  enumStr.replace( ":", "_" ); // xsd:int -> xsd_int  (testcase: salesforce-partner.wsdl)
  return enumStr;
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
      code += "QRegExp exp( QString::fromLatin1(\"" + type->facetPattern() + "\") );";
    code += "rangeOk = rangeOk && exp.exactMatch( " + variableName + " );";

    parentClass.addInclude( "QRegExp" );
  }

  return code;
}
