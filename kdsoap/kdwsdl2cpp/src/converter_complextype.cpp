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

#include "converter.h"

#include <QDebug>

using namespace KWSDL;

void Converter::convertComplexType( const XSD::ComplexType *type )
{
  if ( type->isEmpty() )
    return;
  const QString typeName( mTypeMap.localType( type->qualifiedName() ) );
  KODE::Class newClass( typeName );
  newClass.setUseSharedData( true, "d_ptr" /*avoid clash with possible d() method */ );

  const bool doDebug = (qgetenv("KDSOAP_TYPE_DEBUG").toInt());
  if (doDebug)
      qDebug() << "Generating complex type" << typeName;

  // subclass handling
  if ( !type->baseTypeName().isEmpty() ) { // this class extends something
    /**
     * A class can't subclass basic type (e.g. int, unsigned char), so we
     * add setValue() and value() methods to access the base type.
     */
    if ( mTypeMap.isBasicType( type->baseTypeName() ) ) {
      const QName baseName = type->baseTypeName();
      const QString typeName = mTypeMap.localType( baseName );

      // include header
      newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( baseName ) );
      newClass.addHeaderIncludes( mTypeMap.headerIncludes( baseName ) );

      // member variables
      KODE::MemberVariable variable( "value", typeName );
      newClass.addMemberVariable( variable );

      const QString variableName = "d_ptr->" + variable.name();

      // setter method
      KODE::Function setter( "setValue", "void" );
      setter.addArgument( typeName + " value" );
      setter.setBody( variableName + " = value;" );

      // getter method
      KODE::Function getter( "value", typeName );
      getter.setBody( "return " + variableName + ';' );
      getter.setConst( true );

      // convenience constructor
      KODE::Function conctor( upperlize( newClass.name() ) );
      conctor.addArgument( typeName + " value" );
      conctor.setBody( variableName + " = value;" );

      // type operator
      KODE::Function op( "operator const " + typeName );
      op.setBody( "return " + variableName + ';' );
      op.setConst( true );

      newClass.addFunction( conctor );
      newClass.addFunction( op );
      newClass.addFunction( setter );
      newClass.addFunction( getter );
    } else if ( type->baseTypeName().localName() == "Array" ) { // handle soap array
      // this is handled in the attribute section
    } else {
      if ( type->baseTypeName() != XmlAnyType ) {
        QString baseName = mTypeMap.localType( type->baseTypeName() );
        newClass.addBaseClass( KODE::Class( baseName ) );
      }
    }
  }

  if ( !type->documentation().isEmpty() )
    newClass.setDocs( type->documentation().simplified() );

  // elements
  const XSD::Element::List elements = type->elements();
  Q_FOREACH( const XSD::Element &elemIt, elements ) {

      //qDebug() << elemIt.name() << elemIt.qualifiedName() << elemIt.type();
      if (elemIt.type().isEmpty()) {
          qDebug() << "ERROR: Element with no type:" << elemIt.name() << "(skipping)";
          Q_ASSERT(false);
          continue;
      }

    QString typeName = mTypeMap.localType( elemIt.type() );

    if (typeName != "void") // void means empty element, probably just here for later extensions (testcase: SetPasswordResult in salesforce)
    {

        QString inputTypeName = mTypeMap.localInputType( elemIt.type(), QName() );

        if ( elemIt.maxOccurs() > 1 ) {
            typeName = "QList<" + typeName + ">";
            inputTypeName = "const " + typeName + "&";
        }

        // member variables
        KODE::MemberVariable variable( elemIt.name(), typeName );
        newClass.addMemberVariable( variable );

        const QString variableName = "d_ptr->" + variable.name();

        const QString upperName = upperlize( elemIt.name() );
        const QString lowerName = lowerlize( elemIt.name() );

        // setter method
        KODE::Function setter( "set" + upperName, "void" );
        setter.addArgument( inputTypeName + ' ' + mNameMapper.escape( lowerName ) );
        setter.setBody( variableName + " = " + mNameMapper.escape( lowerName ) + ';' );

        // getter method
        KODE::Function getter( mNameMapper.escape( lowerName ), typeName );
        getter.setBody( "return " + variableName + ';' );
        getter.setConst( true );

        newClass.addFunction( setter );
        newClass.addFunction( getter );
    }

    // include header
    newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( elemIt.type() ) );
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( elemIt.type() ) );
    if ( elemIt.maxOccurs() > 1 )
      newClass.addHeaderIncludes( QStringList( "QList" ) );
  }

  // attributes
  XSD::Attribute::List attributes = type->attributes();
  Q_FOREACH(const XSD::Attribute& attribute, attributes) {
    QString typeName, inputTypeName;

    const bool isArray = !attribute.arrayType().isEmpty();
    if ( isArray ) {
      const QString arrayTypeName = mTypeMap.localType( attribute.arrayType() );
      //qDebug() << "array of" << attribute.arrayType() << "->" << arrayTypeName;
      typeName = "QList<" + arrayTypeName + ">";
      newClass.addInclude(QString(), arrayTypeName); // add forward declaration
      inputTypeName = "const " + typeName + '&';
    } else {
      typeName = mTypeMap.localType( attribute.type() );
      inputTypeName = mTypeMap.localInputType( attribute.type(), QName() );
      qDebug() << "Attribute" << attribute.name();
    }

    // member variables
    KODE::MemberVariable variable( attribute.name(), typeName );
    newClass.addMemberVariable( variable );
    const QString variableName = "d_ptr->" + variable.name();

    QString upperName = upperlize( attribute.name() );
    QString lowerName = lowerlize( attribute.name() );

    // setter method
    KODE::Function setter( "set" + upperName, "void" );
    setter.addArgument( inputTypeName + ' ' + mNameMapper.escape( lowerName ) );
    setter.setBody( variableName + " = " + mNameMapper.escape( lowerName ) + ';' );

    // getter method
    KODE::Function getter( mNameMapper.escape( lowerName ), typeName );
    getter.setBody( "return " + variableName + ';' );
    getter.setConst( true );

    newClass.addFunction( setter );
    newClass.addFunction( getter );

    // include header
    newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( attribute.type() ) );
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( attribute.type() ) );
    if ( isArray )
      newClass.addHeaderIncludes( QStringList() << "QList" );
  }

  createComplexTypeSerializer( newClass, type );

  KODE::Function ctor( upperlize( newClass.name() ) );
  newClass.addFunction( ctor );

  KODE::Function dtor( '~' + upperlize( newClass.name() ) );
  newClass.addFunction( dtor );

  mClasses.append( newClass );
}

// Helper method for the generation of the serialize() method
static KODE::Code appendElementArg( const TypeMap& typeMap, const QName& type, const QString& name, const QString& localVariableName, const QByteArray& varName )
{
    KODE::Code block;
    QString value;
    //qDebug() << "appendElementArg: type=" << type << "isBuiltin=" << typeMap.isBuiltinType(type);
    if ( typeMap.isTypeAny( type ) ) {
        block += varName + ".append(" + localVariableName + ");";
    } else {
        if ( typeMap.isBuiltinType( type ) ) {
            value = localVariableName;
        } else {
            value = localVariableName + ".serialize()";
        }
        // TODO a repository of ready namespace QStrings somewhere, to speed things up...
        block += varName + ".append(KDSoapValue(QString::fromLatin1(\"" + name + "\"), " + value
                 + ", QString::fromLatin1(\"" + type.nameSpace() + "\"), QString::fromLatin1(\"" + type.localName() + "\")));";
    }
    return block;
}

// Helper method for the generation of the deserialize() method
static KODE::Code demarshalNameTest( TypeMap& typeMap, const QName& type, const QString& tagName, bool *first )
{
    KODE::Code demarshalCode;
    if ( typeMap.isTypeAny( type ) ) {
        demarshalCode += QString::fromLatin1(*first? "" : "else ") + '{';
    } else {
        demarshalCode += QString::fromLatin1(*first? "" : "else ") + "if (name == QLatin1String(\"" + tagName + "\")) {";
    }
    *first = false;
    return demarshalCode;
}

// Helper method for the generation of the deserialize() method
static KODE::Code demarshalVar( TypeMap& typeMap, const QName& type, const QString& variableName, const QString& typeName )
{
    KODE::Code code;
    if ( typeMap.isTypeAny( type ) ) {
        Q_ASSERT(false);
        code += variableName + " = val.value();";
    } else if ( typeMap.isBuiltinType( type ) ) {
        code += variableName + " = val.value().value<" + typeName + ">();";
    } else if ( typeMap.isComplexType( type ) ) {
        code += variableName + ".deserialize(val.childValues());";
    } else {
        code += variableName + ".deserialize(val.value());";
    }
    return code;
}

static KODE::Code demarshalArrayVar( TypeMap& typeMap, const QName& type, const QString& variableName, const QString& typeName )
{
    KODE::Code code;
    if ( typeMap.isTypeAny( type ) ) { // KDSoapValue doesn't support temp vars. This special-casing is ugly though.
        code += variableName + ".append(val);";
    } else {
        // we need a temp var in case of deserialize()
        // [TODO: we could merge demarshalVar into this code, to avoid the temp var in other cases]
        QString tempVar;
        if (variableName.startsWith("d_ptr->"))
            tempVar = variableName.mid(7) + "Temp";
        else
            tempVar = variableName + "Temp";
        code += typeName + " " + tempVar + ";";
        code.addBlock( demarshalVar( typeMap, type, tempVar, typeName ) );
        code += variableName + ".append(" + tempVar + ");";
    }
    return code;
}

void Converter::createComplexTypeSerializer( KODE::Class& newClass, const XSD::ComplexType *type )
{
    KODE::Function serializeFunc( "serialize", "QVariant" );
    serializeFunc.setConst( true );

    KODE::Function deserializeFunc( "deserialize", "void" );
    deserializeFunc.addArgument( "const KDSoapValueList& args" );

    KODE::Code marshalCode, demarshalCode;

    if ( type->baseTypeName() != XmlAnyType && !type->baseTypeName().isEmpty() && !type->isArray() ) {
        qDebug() << "TODO: handle marshalling/demarshalling of base types";
    }

    marshalCode += "KDSoapValueList args;";

    // elements
    const XSD::Element::List elements = type->elements();
    const XSD::Attribute::List attributes = type->attributes();

    if ( !elements.isEmpty() ) {
        demarshalCode += "for (int argNr = 0; argNr < args.count(); ++argNr) {";
        demarshalCode.indent();
        demarshalCode += "const KDSoapValue& val = args.at(argNr);";
        demarshalCode += "const QString name = val.name();";
    }

    bool first = true;
    Q_FOREACH( const XSD::Element& elem, elements ) {

        const QString elemName = elem.name();
        const QString typeName = mTypeMap.localType( elem.type() );
        Q_ASSERT(!typeName.isEmpty());

        if ( typeName == "void" )
            continue;

        KODE::MemberVariable variable( elemName, typeName ); // was already added; this is just for the naming
        const QString variableName = "d_ptr->" + variable.name();

        demarshalCode.addBlock( demarshalNameTest( mTypeMap, elem.type(), elemName, &first ) );
        demarshalCode.indent();

        if ( elem.maxOccurs() > 1 ) {
            //const QString typePrefix = mNSManager.prefix( elem.type().nameSpace() );

            marshalCode += "for (int i = 0; i < " + variableName + ".count(); ++i) {";
            marshalCode.indent();
            marshalCode.addBlock( appendElementArg( mTypeMap, elem.type(), elem.name(), variableName + ".at(i)", "args" ) );
            marshalCode.unindent();
            marshalCode += '}';

            demarshalCode.addBlock( demarshalArrayVar( mTypeMap, elem.type(), variableName, typeName ) );
        } else {
            marshalCode.addBlock( appendElementArg( mTypeMap, elem.type(), elem.name(), variableName, "args" ) );
            demarshalCode.addBlock( demarshalVar( mTypeMap, elem.type(), variableName, typeName ) );
        }

        demarshalCode.unindent();
        demarshalCode += "}";
    } // end: for each element

    if ( !elements.isEmpty() ) {
        demarshalCode.unindent();
        demarshalCode += "}";
    }

    if ( !attributes.isEmpty() ) {

        marshalCode += "KDSoapValueList attribs;";

        demarshalCode += "const QList<KDSoapValue> attribs = args.attributes();";
        demarshalCode += "for (int attrNr = 0; attrNr < attribs.count(); ++attrNr) {";
        demarshalCode.indent();
        demarshalCode += "const KDSoapValue& val = attribs.at(attrNr);";
        demarshalCode += "const QString name = val.name();";

        bool first = true;
        Q_FOREACH( const XSD::Attribute& attribute, attributes ) {
            const QString attrName = attribute.name();
            KODE::MemberVariable variable( attrName, "doesnotmatter" ); // was already added; this is just for the naming
            const QString variableName = "d_ptr->" + variable.name();

            demarshalCode.addBlock( demarshalNameTest( mTypeMap, attribute.type(), attrName, &first ) );
            demarshalCode.indent();

            const bool isArray = !attribute.arrayType().isEmpty();
            if (isArray) {
                // qDebug() << "attribute:" << attribute.name() << attribute.type() << attribute.arrayType();

                const QString typeName = mTypeMap.localType( attribute.arrayType() );

                marshalCode += "args.setArrayType(QString::fromLatin1(\"" + attribute.arrayType().nameSpace() + "\"), QString::fromLatin1(\"" + attribute.arrayType().localName() + "\"));";
                marshalCode += "for (int i = 0; i < " + variableName + ".count(); ++i) {";
                marshalCode.indent();
                marshalCode.addBlock( appendElementArg( mTypeMap, attribute.arrayType(), "item", variableName + ".at(i)", "args" ) );
                marshalCode.unindent();
                marshalCode += '}';

                demarshalCode.addBlock( demarshalArrayVar( mTypeMap, attribute.arrayType(), variableName, typeName ) );
            } else {
                marshalCode.addBlock( appendElementArg( mTypeMap, attribute.type(), attribute.name(), variableName, "attribs") );

                const QString typeName = mTypeMap.localType( attribute.type() );
                Q_ASSERT(!typeName.isEmpty());
                demarshalCode.addBlock( demarshalVar( mTypeMap, attribute.type(), variableName, typeName ) );
            }

            demarshalCode.unindent();
            demarshalCode += "}";
        }
        marshalCode += "args.attributes() += attribs;";

        demarshalCode.unindent();
        demarshalCode += "}";
    }

    marshalCode += "return QVariant::fromValue(args);";

    serializeFunc.setBody( marshalCode );
    newClass.addFunction( serializeFunc );

    deserializeFunc.setBody( demarshalCode );
    newClass.addFunction( deserializeFunc );
}
