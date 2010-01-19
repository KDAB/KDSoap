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

#include <common/nsmanager.h>

#include <QDebug>
#include "typemap.h"

using namespace KWSDL;

static QString adaptLocalTypeName( const QString &str )
{
  QString result( str );
  result[ 0 ] = result[ 0 ].toUpper();

  if ( result.endsWith( "[]" ) )
    result.truncate( result.length() - 2 );

  return result;
}

TypeMap::TypeMap()
  : mNSManager( 0 )
{
  addBuiltinType("any", "QString");
  addBuiltinType("anyURI", "QString");
  addBuiltinType("base64Binary", "QByteArray");
  addBuiltinType("binary", "QByteArray");
  addBuiltinType("boolean", "bool");
  addBuiltinType("byte", "char");
  addBuiltinType("date", "QDate");
  addBuiltinType("dateTime", "QDateTime");
  addBuiltinType("decimal", "float");
  addBuiltinType("double", "double");
  // TODO: add duration class
  addBuiltinType("duration", "QString");
  addBuiltinType("float", "float");
  addBuiltinType("integer", "int");
  addBuiltinType("int", "int");
  addBuiltinType("nonPositiveInteger", "int");
  addBuiltinType("language", "QString");
  addBuiltinType("short", "short");
  addBuiltinType("string", "QString");
  addBuiltinType("time", "QTime");
  addBuiltinType("unsignedByte", "unsigned char");
  addBuiltinType("unsignedLong", "unsigned long");
  addBuiltinType("positiveInteger", "unsigned int");
  addBuiltinType("nonNegativeInteger", "unsigned int");
  addBuiltinType("unsignedInt", "unsigned int");
  addBuiltinType("token", "QString");
}

TypeMap::~TypeMap()
{
}

void TypeMap::addBuiltinType(const char *typeName, const char *localType)
{
    Entry entry;
    entry.builtinType = true;
    entry.nameSpace = XMLSchemaURI;
    entry.typeName = typeName;
    entry.localType = localType;
    entry.basicType = !entry.localType.startsWith('Q');
    if ( !entry.basicType ) {
        entry.headers << entry.localType;
        entry.headerIncludes << entry.localType;
    }
    mTypeMap.append( entry );
}

void TypeMap::setNSManager( NSManager *manager )
{
  mNSManager = manager;
}

QList<TypeMap::Entry>::ConstIterator TypeMap::typeEntry( const QName &typeName ) const
{
    QList<Entry>::ConstIterator it;
    for ( it = mTypeMap.constBegin(); it != mTypeMap.constEnd(); ++it ) {
      if ( (*it).typeName == typeName.localName() && (*it).nameSpace == typeName.nameSpace() )
        break;
    }
    return it;
}

bool TypeMap::isBasicType( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  return it != mTypeMap.constEnd() ? (*it).basicType : false;
}

bool TypeMap::isBuiltinType( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  return it != mTypeMap.constEnd() ? (*it).builtinType : false;
}

bool TypeMap::isComplexType( const QName &typeName, const QName& elementName ) const
{
    // Note the use of typeEntry even for the element name;
    // the (now useless?) entry in mElementMap doesn't have complexType set
    QList<Entry>::ConstIterator it = !typeName.isEmpty() ? typeEntry( typeName ) : typeEntry( elementName );
    return it != mTypeMap.constEnd() ? (*it).complexType : false;
}

QString TypeMap::localType( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  if ( it == mTypeMap.constEnd() ) {
      qDebug() << "ERROR: basic type not found:" << typeName.qname();
      return QString();
  }
  return (*it).localType;
}

QString TypeMap::baseType(const QName &typeName) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  if ( it == mTypeMap.constEnd() || (*it).baseType.isEmpty() ) {
      return QString();
  }
  const QName base = (*it).baseType;
  return localType( base );
}

QStringList TypeMap::headers( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  return it != mTypeMap.constEnd() ? (*it).headers : QStringList();
}

QStringList TypeMap::forwardDeclarations( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  return it != mTypeMap.constEnd() ? (*it).forwardDeclarations : QStringList();
}

QStringList TypeMap::headerIncludes( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  return it != mTypeMap.constEnd() ? (*it).headerIncludes : QStringList();
}



QString TypeMap::localTypeForAttribute( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it;
  for ( it = mAttributeMap.constBegin(); it != mAttributeMap.constEnd(); ++it ) {
    if ( (*it).typeName == typeName.localName() && (*it).nameSpace == typeName.nameSpace() )
      return (*it).localType;
  }

  return QString();
}

QStringList TypeMap::headersForAttribute( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it;
  for ( it = mAttributeMap.constBegin(); it != mAttributeMap.constEnd(); ++it ) {
    if ( (*it).typeName == typeName.localName() && (*it).nameSpace == typeName.nameSpace() )
      return (*it).headers;
  }

  return QStringList();
}

QStringList TypeMap::forwardDeclarationsForAttribute( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it;
  for ( it = mAttributeMap.constBegin(); it != mAttributeMap.constEnd(); ++it ) {
    if ( (*it).typeName == typeName.localName() && (*it).nameSpace == typeName.nameSpace() )
      return (*it).forwardDeclarations;
  }

  return QStringList();
}

QList<TypeMap::Entry>::ConstIterator TypeMap::elementEntry( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it;
  for ( it = mElementMap.constBegin(); it != mElementMap.constEnd(); ++it ) {
    if ( (*it).typeName == typeName.localName() && (*it).nameSpace == typeName.nameSpace() )
      break;
  }
  return it;
}

QString TypeMap::localTypeForElement( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = elementEntry( typeName );
  if ( it != mElementMap.constEnd() ) {
      return (*it).localType;
  }

  qDebug() << "TypeMap::localTypeForElement: unknown type" << typeName.qname();
  return QString();
}

#if 0
QStringList TypeMap::headersForElement( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = elementEntry( typeName );
  return it != mElementMap.constEnd() ? (*it).headers : QStringList();
}
#endif

QStringList TypeMap::forwardDeclarationsForElement( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = elementEntry( typeName );
  return it != mElementMap.constEnd() ? (*it).forwardDeclarations : QStringList();
}

void TypeMap::addSchemaTypes( const XSD::Types &types )
{
  Q_ASSERT( mNSManager );

  XSD::SimpleType::List simpleTypes = types.simpleTypes();
  XSD::SimpleType::List::ConstIterator simpleIt;
  for ( simpleIt = simpleTypes.constBegin(); simpleIt != simpleTypes.constEnd(); ++simpleIt ) {
    Entry entry;
    entry.basicType = false;
    entry.builtinType = false;
    entry.nameSpace = (*simpleIt).nameSpace();
    entry.typeName = (*simpleIt).name();
    entry.localType = mNSManager->prefix( entry.nameSpace ).toUpper() + "__" + adaptLocalTypeName( (*simpleIt).name() );
    entry.baseType = (*simpleIt).baseTypeName();
    //qDebug() << entry.baseType.nameSpace() << entry.baseType.localName() << entry.baseType.qname();
    //entry.headers << (*simpleIt).name().toLower() + ".h";
    entry.forwardDeclarations << entry.localType;

    mTypeMap.append( entry );
  }

  XSD::ComplexType::List complexTypes = types.complexTypes();
  XSD::ComplexType::List::ConstIterator complexIt;
  for ( complexIt = complexTypes.constBegin(); complexIt != complexTypes.constEnd(); ++complexIt ) {
    Entry entry;
    entry.basicType = false;
    entry.builtinType = false;
    entry.complexType = true;
    entry.nameSpace = (*complexIt).nameSpace();
    entry.typeName = (*complexIt).name();
    if ( (*complexIt).isEmpty() )
        entry.localType = "void";
    else {
        entry.localType = mNSManager->prefix( entry.nameSpace ).toUpper() + "__" + adaptLocalTypeName( (*complexIt).name() );
        //entry.headers << (*complexIt).name().toLower() + ".h";
        entry.forwardDeclarations << entry.localType;
    }

    mTypeMap.append( entry );
  }

  XSD::Attribute::List attributes = types.attributes();
  XSD::Attribute::List::ConstIterator attrIt;
  for ( attrIt = attributes.constBegin(); attrIt != attributes.constEnd(); ++attrIt ) {
    Entry entry;
    entry.basicType = false;
    entry.builtinType = false;
    entry.nameSpace = (*attrIt).nameSpace();
    entry.typeName = (*attrIt).name();
    entry.localType = mNSManager->prefix( entry.nameSpace ).toUpper() + "__" + adaptLocalTypeName( (*attrIt).name() + "Attribute" );
    entry.headers << (*attrIt).name().toLower() + "attribute.h";
    entry.forwardDeclarations << entry.localType;

    mAttributeMap.append( entry );
  }

  XSD::Element::List elements = types.elements();
  XSD::Element::List::ConstIterator elemIt;
  for ( elemIt = elements.constBegin(); elemIt != elements.constEnd(); ++elemIt ) {
    Entry entry;
    entry.basicType = false;
    entry.builtinType = false;
    entry.nameSpace = (*elemIt).nameSpace();
    entry.typeName = (*elemIt).name();

    QString resolvedType = localType( (*elemIt).type() );
    Q_ASSERT( !resolvedType.isEmpty() );
    entry.localType = resolvedType;

    // The "FooElement" type isn't necessary, we just point to the resolved type
    // directly, this is much simpler.
    /*} else {
      entry.localType = mNSManager->prefix( entry.nameSpace ).toUpper() + "__" + adaptLocalTypeName( (*elemIt).name() + "Element" );
    }*/
    //qDebug() << "Adding TypeMap entry for element" << entry.typeName << resolvedType;
    mElementMap.append( entry );
  }
}

void TypeMap::dump() const
{
  qDebug( "--------------------------------" );
  qDebug( "Types:" );
  for ( int i = 0; i < mTypeMap.count(); ++i ) {
    qDebug( "%s\t%s\t%s\t%s\t%s\t%s",
            qPrintable( mTypeMap[ i ].nameSpace ),
            qPrintable( mTypeMap[ i ].typeName ),
            qPrintable( mTypeMap[ i ].localType ),
            qPrintable( mTypeMap[ i ].headers.join( "," ) ),
            qPrintable( mTypeMap[ i ].headerIncludes.join( "," ) ),
            mTypeMap[ i ].basicType ? "basic" : mTypeMap[i].complexType ? "complex" : "" );
  }

  qDebug( "--------------------------------" );
  qDebug( "Attributes:" );
  for ( int i = 0; i < mAttributeMap.count(); ++i ) {
    qDebug( "%s\t%s\t%s\t%s\t%s\t%s",
            ( mAttributeMap[ i ].basicType ? "yes" : "no" ),
              qPrintable( mAttributeMap[ i ].nameSpace ),
              qPrintable( mAttributeMap[ i ].typeName ),
              qPrintable( mAttributeMap[ i ].localType ),
              qPrintable( mAttributeMap[ i ].headers.join( "," ) ),
              qPrintable( mAttributeMap[ i ].headerIncludes.join( "," ) ) );
  }

  qDebug( "--------------------------------" );
  qDebug( "Elements:" );
  for ( int i = 0; i < mElementMap.count(); ++i ) {
    Q_ASSERT( !mElementMap[ i ].basicType );
    qDebug( "%s\t%s\t%s\t%s\t%s\t%s",
              qPrintable( mElementMap[ i ].nameSpace ),
              qPrintable( mElementMap[ i ].typeName ),
              qPrintable( mElementMap[ i ].localType ),
              qPrintable( mElementMap[ i ].headers.join( "," ) ),
              qPrintable( mElementMap[ i ].headerIncludes.join( "," ) ),
              mTypeMap[ i ].basicType ? "basic" : mTypeMap[i].complexType ? "complex" : "" );
  }
}

QString TypeMap::localType( const QName &typeName, const QName& elementName ) const
{
    if ( !typeName.isEmpty() ) {
        return localType( typeName );
    } else {
        return localTypeForElement( elementName );
    }
}

QString TypeMap::localInputType( const QName &typeName, const QName& elementName ) const
{
    if ( !typeName.isEmpty() ) {
        QList<Entry>::ConstIterator it = typeEntry( typeName );
        if ( it == mTypeMap.constEnd() ) {
            qDebug() << "ERROR: type not found:" << typeName.qname();
            return QString();
        }
        QString argType = (*it).localType;
        if (!(*it).basicType)
            argType = "const " + argType + '&';
        return argType;
    } else {
        QString argType = localTypeForElement( elementName );
        if ( !argType.isEmpty() && argType != "void" )
            argType = "const " + argType + '&';
        return argType;
    }
}
