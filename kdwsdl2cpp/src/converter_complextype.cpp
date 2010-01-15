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

void Converter::convertComplexType( const XSD::ComplexType *type )
{
  const QString typeName( mTypeMap.localType( type->qualifiedName() ) );
  KODE::Class newClass( typeName );

  newClass.addInclude( QString(), "KDSoapValueList" );

  KODE::Code ctorBody;
  KODE::Code dtorBody;

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
      KODE::MemberVariable variable( "value", typeName + '*' );
      newClass.addMemberVariable( variable );

      ctorBody += variable.name() + " = 0;";
      dtorBody += "delete " + variable.name() + "; " + variable.name() + " = 0;";

      // setter method
      KODE::Function setter( "setValue", "void" );
      setter.addArgument( typeName + " *value" );
      setter.setBody( variable.name() + " = value;" );

      // getter method
      KODE::Function getter( "value", typeName + '*' );
      getter.setBody( "return " + variable.name() + ';' );
      getter.setConst( true );

      // convenience constructor
      KODE::Function conctor( upperlize( newClass.name() ) );
      conctor.addArgument( typeName + " *value" );
      conctor.setBody( variable.name() + " = value;" );

      if ( typeName == "QString" ) {
        KODE::Function charctor( upperlize( newClass.name() ) );
        charctor.addArgument( "const char *charValue" );
        charctor.setBody( variable.name() + " = new QString( charValue );" );

        newClass.addFunction( charctor );
      }

      // type operator
      KODE::Function op( "operator const " + typeName + '*' );
      op.setBody( "return " + variable.name() + ';' );
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
  XSD::Element::List elements = type->elements();
  XSD::Element::List::ConstIterator elemIt;
  for ( elemIt = elements.constBegin(); elemIt != elements.constEnd(); ++elemIt ) {
    QString typeName = mTypeMap.localType( (*elemIt).type() );

    if ( (*elemIt).maxOccurs() > 1 )
      typeName = "QList<" + typeName + ">";

    // member variables
    KODE::MemberVariable variable( (*elemIt).name(), typeName );
    newClass.addMemberVariable( variable );

    //ctorBody += variable.name() + " = 0;";
    //if ( (*elemIt).maxOccurs() > 1 ) {
      //dtorBody += "qDeleteAll( *" + variable.name() + " );";
      //dtorBody += variable.name() + ".clear();";
    //}
    //dtorBody += "delete " + variable.name() + "; " + variable.name() + " = 0;";

    const QString upperName = upperlize( (*elemIt).name() );
    const QString lowerName = lowerlize( (*elemIt).name() );

    // setter method
    KODE::Function setter( "set" + upperName, "void" );
    setter.addArgument( mTypeMap.inputType( typeName, false ) + ' ' + mNameMapper.escape( lowerName ) );
    setter.setBody( variable.name() + " = " + mNameMapper.escape( lowerName ) + ';' );

    // getter method
    KODE::Function getter( mNameMapper.escape( lowerName ), typeName );
    getter.setBody( "return " + variable.name() + ';' );
    getter.setConst( true );

    newClass.addFunction( setter );
    newClass.addFunction( getter );

    // include header
    newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( (*elemIt).type() ) );
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( (*elemIt).type() ) );
    if ( (*elemIt).maxOccurs() > 1 )
      newClass.addHeaderIncludes( QStringList( "QList" ) );
  }

  // attributes - TODO test and port
  XSD::Attribute::List attributes = type->attributes();
  XSD::Attribute::List::ConstIterator attrIt;
  for ( attrIt = attributes.constBegin(); attrIt != attributes.constEnd(); ++attrIt ) {
    QString typeName;

    bool isArray = !(*attrIt).arrayType().isEmpty();
    if ( isArray )
      typeName = "QList<" + mTypeMap.localType( (*attrIt).arrayType() ) + "*>";
    else
      typeName = mTypeMap.localType( (*attrIt).type() );

    // member variables
    KODE::MemberVariable variable( (*attrIt).name(), typeName + '*' );
    newClass.addMemberVariable( variable );

    ctorBody += variable.name() + " = 0;";
    if ( isArray ) {
      dtorBody += "qDeleteAll( *" + variable.name() + " );";
      dtorBody += variable.name() + "->clear();";
    }
    dtorBody += "delete " + variable.name() + "; " + variable.name() + " = 0;";

    QString upperName = upperlize( (*attrIt).name() );
    QString lowerName = lowerlize( (*attrIt).name() );

    // setter method
    KODE::Function setter( "set" + upperName, "void" );
    setter.addArgument( typeName + " *" + mNameMapper.escape( lowerName ) );
    setter.setBody( variable.name() + " = " + mNameMapper.escape( lowerName ) + ';' );

    // getter method
    KODE::Function getter( mNameMapper.escape( lowerName ), typeName + '*' );
    getter.setBody( "return " + variable.name() + ';' );
    getter.setConst( true );

    newClass.addFunction( setter );
    newClass.addFunction( getter );

    // include header
    newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( (*attrIt).type() ) );
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( (*attrIt).type() ) );
    if ( isArray )
      newClass.addHeaderIncludes( QStringList( "QList" ) );
  }

  createComplexTypeSerializer( newClass, type );

  KODE::Function ctor( upperlize( newClass.name() ) );
  ctor.setBody( ctorBody );
  newClass.addFunction( ctor );

  KODE::Function dtor( '~' + upperlize( newClass.name() ) );
  dtor.setBody( dtorBody );
  newClass.addFunction( dtor );

  mClasses.append( newClass );
}

void Converter::createComplexTypeSerializer( KODE::Class& newClass, const XSD::ComplexType *type )
{
    KODE::Function serializeFunc( "serialize", "void" );
    serializeFunc.addArgument( "KDSoapValueList& args" );
    serializeFunc.setConst( true );

    KODE::Function deserializeFunc( "deserialize", "void" );
    deserializeFunc.addArgument( "const KDSoapValueList& args" );

    KODE::Code marshalCode, demarshalCode;

    if ( type->baseTypeName() != XmlAnyType && !type->baseTypeName().isEmpty() && !type->isArray() ) {
        qDebug() << "TODO: handle marshalling/demarshalling of base types";
    }

    // elements
    XSD::Element::List elements = type->elements();

    if ( !elements.isEmpty() ) {
        demarshalCode += "for (int argNr = 0; argNr < args.count(); ++argNr) {";
        demarshalCode.indent();
        demarshalCode += "const KDSoapValue& val = args.at(argNr);";
        demarshalCode += "const QString name = val.name();";
        demarshalCode += "const QVariant value = val.value();";
    }

    Q_FOREACH( const XSD::Element& elem, elements ) {

        const QString typeName = mTypeMap.localType( elem.type() );
        KODE::MemberVariable variable( elem.name(), typeName ); // was already added; this is just for the naming
        //const QString upperName = upperlize( elem.name() );
        //const QString lowerName = lowerlize( elem.name() );

        demarshalCode += "if (name == \"" + elem.name() + "\") {";
        demarshalCode.indent();

        if ( elem.maxOccurs() > 1 ) {
            //const QString typePrefix = mNSManager.prefix( elem.type().nameSpace() );

            marshalCode += "for (int i = 0; i < " + variable.name() + ".count(); ++i) {";
            marshalCode.indent();
            marshalCode += "args.append(KDSoapValue(\"" + elem.name() + "\", " + variable.name() + ".at(i)));";
            marshalCode.unindent();
            marshalCode += '}';

            demarshalCode += variable.name() + ".append(value.value<" + typeName + ">());";
        } else {
            marshalCode += "args.append(KDSoapValue(\"" + elem.name() + "\", " + variable.name() + "));";

            demarshalCode += variable.name() + " = value.value<" + typeName + ">();";
        }

        demarshalCode.unindent();
        demarshalCode += "}";
    }

    if ( !type->attributes().isEmpty() ) {
        qDebug() << "TODO: handling marshalling of attributes";
    }

    serializeFunc.setBody( marshalCode );
    newClass.addFunction( serializeFunc );

    if ( !elements.isEmpty() ) {
        demarshalCode.unindent();
        demarshalCode += "}";
    }

    deserializeFunc.setBody( demarshalCode );
    newClass.addFunction( deserializeFunc );

    Q_UNUSED(type);
#ifdef KDAB_DELETED
  const QString typeName = mTypeMap.localType( type->qualifiedName() );
  const QString baseType = mTypeMap.localType( type->baseTypeName() );

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

  KODE::Code marshalCode, demarshalCode, demarshalStartCode, demarshalFinalCode;

  marshalCode += "if ( !value )";
  marshalCode.indent();
  marshalCode += "return;";
  marshalCode.unindent();
  marshalCode.newLine();

  demarshalStartCode += "if ( !value )";
  demarshalStartCode.indent();
  demarshalStartCode += "return;";
  demarshalStartCode.unindent();
  demarshalStartCode.newLine();

  // include header
  mSerializer.addIncludes( QStringList(), mTypeMap.forwardDeclarations( type->qualifiedName() ) );

  marshalCode += "QDomElement parentElement;";
  marshalCode += "if ( !name.isEmpty() ) {";
  marshalCode.indent();

  const QString typePrefix = mNSManager.prefix( type->qualifiedName().nameSpace() );
  marshalCode += "QDomElement root = doc.createElement( noNamespace ? name : \"" + typePrefix + ":\" + name );";
  marshalCode += "root.setAttribute( \"" + mNSManager.schemaInstancePrefix() + ":type\", \"" +
                 typePrefix + ":" + typeName + "\" );";
  marshalCode += "parent.appendChild( root );";
  marshalCode += "parentElement = root;";
  marshalCode.unindent();
  marshalCode += "} else {";
  marshalCode.indent();
  marshalCode += "parentElement = parent;";
  marshalCode.unindent();
  marshalCode += '}';
  marshalCode.newLine();

  // handle marshalling of base types
  if ( type->baseTypeName() != XmlAnyType && !type->baseTypeName().isEmpty() && !type->isArray() ) {
    marshalCode += "// marshall base type";
    marshalCode += "Serializer::marshal( doc, parentElement, QString(), (" + baseType + "*)value, noNamespace );";
  }

  // handle demarshalling of base types
  if ( type->baseTypeName() != XmlAnyType && !type->baseTypeName().isEmpty() && !type->isArray() ) {
    demarshalCode += "// demarshall base type";
    demarshalCode += "Serializer::demarshal( parent, (" + baseType + "*)value );";
    demarshalCode.newLine();
  }

  demarshalCode += "QDomNode node = parent.firstChild();";
  demarshalCode += "while ( !node.isNull() ) {";
  demarshalCode.indent();
  demarshalCode += "QDomElement element = node.toElement();";
  demarshalCode += "if ( !element.isNull() ) {";
  demarshalCode.indent();

  // elements
  XSD::Element::List elements = type->elements();
  XSD::Element::List::ConstIterator elemIt;
  for ( elemIt = elements.constBegin(); elemIt != elements.constEnd(); ++elemIt ) {
    const QString typeName = mTypeMap.localType( (*elemIt).type() );

    QString upperName = upperlize( (*elemIt).name() );
    QString lowerName = lowerlize( (*elemIt).name() );

    KODE::Function setter( "set" + upperName, "void" );
    KODE::Function getter( mNameMapper.escape( lowerName ), typeName + '*' );

    if ( (*elemIt).maxOccurs() > 1 ) {
      const QString typePrefix = mNSManager.prefix( (*elemIt).type().nameSpace() );

      marshalCode += '{';
      marshalCode.indent();
      marshalCode += "const QList<" + typeName + "*> *list = value->" + mNameMapper.escape( lowerName ) + "();";
      marshalCode.newLine();
      marshalCode += "QList<" + typeName + "*>::ConstIterator it;";
      marshalCode += "for ( it = list->begin(); it != list->end(); ++it ) {";
      marshalCode.indent();
      marshalCode += "Serializer::marshal( doc, parentElement, \"" + typePrefix + ":" + (*elemIt).name() + "\", *it, false );";
      marshalCode.unindent();
      marshalCode += '}';
      marshalCode.unindent();
      marshalCode += '}';

      const QString listName = mNameMapper.escape( lowerName ) + "List";

      demarshalStartCode += "QList<" + typeName + "*> *" + listName + " = new QList<" + typeName + "*>();";

      demarshalCode += "if ( element.tagName() == \"" + (*elemIt).name() + "\" ) {";
      demarshalCode.indent();
      demarshalCode += typeName + " *item = new " + typeName + "();";
      demarshalCode += "Serializer::demarshal( element, item );";
      demarshalCode += listName + "->append( item );";
      demarshalCode.unindent();
      demarshalCode += '}';

      demarshalFinalCode += "value->" + setter.name() + "( " + listName + " );";
    } else {
      marshalCode += "Serializer::marshal( doc, parentElement, \"" + (*elemIt).name() + "\", value->" + getter.name() + "(), false );";

      demarshalCode += "if ( element.tagName() == \"" + (*elemIt).name() + "\" ) {";
      demarshalCode.indent();
      demarshalCode += typeName + " *item = new " + typeName + "();";
      demarshalCode += "Serializer::demarshal( element, item );";
      demarshalCode += "value->" + setter.name() + "( item );";
      demarshalCode.unindent();
      demarshalCode += '}';
    }
  }

  // attributes
  XSD::Attribute::List attributes = type->attributes();
  XSD::Attribute::List::ConstIterator attrIt;
  for ( attrIt = attributes.constBegin(); attrIt != attributes.constEnd(); ++attrIt ) {
    QString upperName = upperlize( (*attrIt).name() );
    QString lowerName = lowerlize( (*attrIt).name() );

    KODE::Function setter( "set" + upperName, "void" );

    bool isArray = !(*attrIt).arrayType().isEmpty();
    if ( isArray ) {
      const QString typeName = mTypeMap.localType( (*attrIt).arrayType() );
      const QString typePrefix = mNSManager.prefix( (*attrIt).arrayType().nameSpace() );

      marshalCode += '{';
      marshalCode.indent();
      marshalCode += "const QList<" + typeName + "*> *list = value->" + mNameMapper.escape( lowerName ) + "();";
      marshalCode.newLine();

      marshalCode += "QDomElement element = doc.createElement( noNamespace ? name : \"" + typePrefix + ":\" + name );";
      marshalCode += "element.setAttribute( \"" + mNSManager.schemaInstancePrefix() + ":type\", \"" +
                      mNSManager.soapEncPrefix() + ":Array\" );";
      marshalCode += "element.setAttribute( \"" + mNSManager.soapEncPrefix() +
                     ":arrayType\", \"" + typePrefix + ":" + typeName + "[\" + QString::number( list->count() ) + \"]\" );";
      marshalCode += "parentElement.appendChild( element );";
      marshalCode.newLine();
      marshalCode += "QList<" + typeName + "*>::ConstIterator it;";
      marshalCode += "for ( it = list->begin(); it != list->end(); ++it ) {";
      marshalCode.indent();
      marshalCode += "Serializer::marshal( doc, element, \"item\", *it, false );";
      marshalCode.unindent();
      marshalCode += '}';
      marshalCode.unindent();
      marshalCode += '}';

      const QString listName = mNameMapper.escape( lowerName ) + "List";
      // TODO: prepend the code somehow

      demarshalStartCode += "QList<" + typeName + "*> *" + listName + " = new QList<" + typeName + "*>();";

      demarshalCode.indent();
      demarshalCode += typeName + " *item = new " + typeName + "();";
      demarshalCode += "Serializer::demarshal( element, item );";
      demarshalCode += listName + "->append( item );";
      demarshalCode.unindent();

      demarshalFinalCode += "value->" + setter.name() + "( " + listName + " );";
    } else {
      const QString typeName = mTypeMap.localType( (*attrIt).type() );

      marshalCode += "parentElement.setAttribute( \"" + (*attrIt).name() + "\", "
                     "Serializer::marshalValue( value->" + mNameMapper.escape( lowerName ) + "() ) );";
      marshalCode.newLine();

      demarshalCode += "if ( element.hasAttribute( \"" + (*attrIt).name() + "\" ) ) {";
      demarshalCode.indent();
      demarshalCode += typeName + " *item = new " + typeName + "();";
      demarshalCode += "Serializer::demarshalValue( element.attribute( \"" + (*attrIt).name() + "\" ), item );";
      demarshalCode += "value->" + setter.name() + "( item );";
      demarshalCode.unindent();
      demarshalCode += '}';
    }
  }

  demarshalCode.unindent();
  demarshalCode += '}';
  demarshalCode += "node = node.nextSibling();";
  demarshalCode.unindent();
  demarshalCode += '}';
  demarshalCode.newLine();

  demarshalCode += demarshalFinalCode;
  demarshalStartCode.newLine();
  demarshalStartCode += demarshalCode;

  marshal.setBody( marshalCode );
  mSerializer.addFunction( marshal );

  demarshal.setBody( demarshalStartCode );
  mSerializer.addFunction( demarshal );
#endif
}
