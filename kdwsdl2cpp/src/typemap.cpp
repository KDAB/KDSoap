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
  // see http://www.w3.org/TR/xmlschema-2
  addBuiltinType("any", "KDSoapValue");
  addBuiltinType("anyType", "KDSoapValue");
  addBuiltinType("anySimpleType", "QVariant");
  addBuiltinType("anyURI", "QString");
  addBuiltinType("base64Binary", "QByteArray");
  addBuiltinType("boolean", "bool");
  addBuiltinType("byte", "signed char"); // 8 bit, signed
  addBuiltinType("date", "QDate");
  addBuiltinType("dateTime", "KDDateTime");
  addBuiltinType("decimal", "float");
  addBuiltinType("double", "double");
  // TODO: add duration class
  addBuiltinType("duration", "QString");
  addBuiltinType("float", "float");
  addBuiltinType("hexBinary", "QByteArray");
  addBuiltinType("integer", "qint64");
  addBuiltinType("int", "int"); // 32 bits
  addBuiltinType("nonPositiveInteger", "qint64");
  addBuiltinType("language", "QString");
  addBuiltinType("long", "qint64"); // 64 bits
  addBuiltinType("short", "int"); // 16 bits. But QVariant doesn't support short.
  addBuiltinType("string", "QString");
  addBuiltinType("time", "QTime");
  addBuiltinType("unsignedByte", "unsigned char");
  addBuiltinType("unsignedShort", "unsigned int"); // 16 bits, 0-65535. But QVariant doesn't support short.
  addBuiltinType("unsignedLong", "quint64"); // 64 bits
  addBuiltinType("positiveInteger", "quint64"); // unbounded (1 to inf)
  addBuiltinType("nonPositiveInteger", "qint64"); // unbounded (-inf to 0)
  addBuiltinType("negativeInteger", "qint64"); // unbounded (-inf to -1)
  addBuiltinType("nonNegativeInteger", "quint64"); // unbounded (0 to inf)
  addBuiltinType("unsignedInt", "unsigned int"); // 32 bits
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
    entry.basicType = !entry.localType.startsWith('Q') && !entry.localType.startsWith('K');
    if ( !entry.basicType ) {
        QString header = entry.localType;
        if (entry.localType.startsWith("KD")) {
            header += ".h";
            header.prepend("KDSoapClient/");
        } else if (entry.localType.startsWith("Q"))
            header.prepend("QtCore/");
        entry.headers << header;
        entry.headerIncludes << header;
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

bool TypeMap::isComplexType( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  return it != mTypeMap.constEnd() ? (*it).complexType : false;
}

bool TypeMap::isBuiltinType( const QName &typeName, const QName& elementName ) const
{
    if (!typeName.isEmpty()) {
        return isBuiltinType( typeName );
    } else {
        QList<Entry>::ConstIterator it = elementEntry( elementName );
        return it != mElementMap.constEnd() ? (*it).builtinType : false;
    }
}

bool TypeMap::isComplexType( const QName &typeName, const QName& elementName ) const
{
    if (!typeName.isEmpty()) {
        return isComplexType( typeName );
    } else {
        QList<Entry>::ConstIterator it = elementEntry( elementName );
        return it != mElementMap.constEnd() ? (*it).complexType : false;
    }
}

QString TypeMap::localType( const QName &typeName ) const
{
  QList<Entry>::ConstIterator it = typeEntry( typeName );
  if ( it == mTypeMap.constEnd() ) {
      qDebug() << "ERROR: basic type not found:" << typeName;
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

QString TypeMap::localTypeForElement( const QName &elementName ) const
{
  QList<Entry>::ConstIterator it = elementEntry( elementName );
  if ( it != mElementMap.constEnd() ) {
      return (*it).localType;
  }

  qDebug() << "TypeMap::localTypeForElement: unknown type" << elementName;
  return QString();
}

QName TypeMap::typeForElement( const QName &elementName ) const
{
  QList<Entry>::ConstIterator it = elementEntry( elementName );
  if ( it != mElementMap.constEnd() ) {
      return (*it).baseType;
  }

  qDebug() << "TypeMap::typeForElement: unknown type" << elementName;
  return QName();
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

static QString prefixNamespace( const QString& input, const QString& ns )
{
  if (ns.isEmpty())
      return input;
  return ns + QLatin1String("::") + input;
}

void TypeMap::addSchemaTypes( const XSD::Types &types, const QString& ns )
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
    entry.localType = prefixNamespace( mNSManager->prefix( entry.nameSpace ).toUpper() + "__" + adaptLocalTypeName( (*simpleIt).name() ), ns );
    entry.baseType = (*simpleIt).baseTypeName();
    //qDebug() << entry.baseType.nameSpace() << entry.baseType.localName() << entry.baseType;
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

    //qDebug() << "TypeMap: adding complex type" << entry.nameSpace << entry.typeName;

    // Keep empty complex types, useful for document mode.
    /*if ( (*complexIt).isEmpty() )
        entry.localType = "void";
    else*/ {
        entry.localType = prefixNamespace( mNSManager->prefix( entry.nameSpace ).toUpper() + "__" + adaptLocalTypeName( (*complexIt).name() ), ns );
        if ((*complexIt).isConflicting()) {
            entry.localType += (*complexIt).isAnonymous() ? "Element" : "Type";
        }
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
    entry.localType = prefixNamespace( mNSManager->prefix( entry.nameSpace ).toUpper() + "__" + adaptLocalTypeName( (*attrIt).name() + "Attribute" ), ns );
    entry.headers << (*attrIt).name().toLower() + "attribute.h";
    entry.forwardDeclarations << entry.localType;

    mAttributeMap.append( entry );
  }

  const XSD::Element::List elements = types.elements();
  Q_FOREACH( const XSD::Element& elemIt, elements ) {
    Entry entry;
    entry.nameSpace = elemIt.nameSpace();
    entry.typeName = elemIt.name();

    QName type = elemIt.type();
    if ( type.isEmpty() ) {
        qDebug() << "ERROR: element without type" << elemIt.nameSpace() << elemIt.name();
    }

    // Resolve to localType(type)
    QList<Entry>::ConstIterator it = typeEntry(type);
    if ( it == mTypeMap.constEnd() ) {
        qDebug() << "ERROR: basic type not found:" << type;
        Q_ASSERT(0);
        continue;
    }
    const QString resolvedType = (*it).localType;
    Q_ASSERT( !resolvedType.isEmpty() );
    entry.localType = resolvedType;
    entry.basicType = (*it).basicType;
    entry.builtinType = (*it).builtinType;
    entry.complexType = (*it).complexType;
    entry.baseType = type;

    // The "FooElement" type isn't necessary, we just point to the resolved type
    // directly, this is much simpler.
    /*} else {
      entry.localType = mNSManager->prefix( entry.nameSpace ).toUpper() + "__" + adaptLocalTypeName( elemIt.name() + "Element" );
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
            qPrintable( mTypeMap[ i ].dumpBools() ) );
  }

  qDebug( "--------------------------------" );
  qDebug( "Attributes:" );
  for ( int i = 0; i < mAttributeMap.count(); ++i ) {
    qDebug( "%s\t%s\t%s\t%s\t%s\t%s",
            qPrintable( mAttributeMap[ i ].nameSpace ),
            qPrintable( mAttributeMap[ i ].typeName ),
            qPrintable( mAttributeMap[ i ].localType ),
            qPrintable( mAttributeMap[ i ].headers.join( "," ) ),
            qPrintable( mAttributeMap[ i ].headerIncludes.join( "," ) ),
            qPrintable( mAttributeMap[ i ].dumpBools() ) );
  }

  qDebug( "--------------------------------" );
  qDebug( "Elements:" );
  for ( int i = 0; i < mElementMap.count(); ++i ) {
    qDebug( "%s\t%s\t%s\t%s\t%s\t%s",
              qPrintable( mElementMap[ i ].nameSpace ),
              qPrintable( mElementMap[ i ].typeName ),
              qPrintable( mElementMap[ i ].localType ),
              qPrintable( mElementMap[ i ].headers.join( "," ) ),
              qPrintable( mElementMap[ i ].headerIncludes.join( "," ) ),
              qPrintable( mElementMap[ i ].dumpBools() ) );
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
    QList<Entry>::ConstIterator it;
    if ( !typeName.isEmpty() ) {
        it = typeEntry( typeName );
        if ( it == mTypeMap.constEnd() ) {
            qDebug() << "ERROR: type not found:" << typeName;
            return QString();
        }
    } else {
        it = elementEntry( elementName );
        if ( it == mElementMap.constEnd() ) {
            qDebug() << "ERROR: element not found:" << elementName;
            return QString();
        }
    }
    QString argType = (*it).localType;
    if (!(*it).basicType && argType != "void")
        argType = "const " + argType + '&';
    return argType;
}

// If the type is represented as a KDSoapValue already, no need to serialize/deserialize it
bool KWSDL::TypeMap::isTypeAny(const QName &typeName) const
{
    return (typeName.nameSpace() == XMLSchemaURI &&
            (typeName.localName() == "any" || typeName.localName() == "anyType"));
}

QString KWSDL::TypeMap::Entry::dumpBools() const
{
    QStringList lst;
    if (basicType)
        lst += "basic";
    if (builtinType)
        lst += "builtin";
    if (complexType)
        lst += "complex";
    return lst.join(",");
}

QString KWSDL::TypeMap::deserializeBuiltin( const QName &typeName, const QName& elementName, const QString& var, const QString& qtTypeName ) const
{
    const QName type = typeName.isEmpty() ? typeForElement(elementName) : typeName;
    if (type.nameSpace() == XMLSchemaURI && type.localName() == "hexBinary") {
        return "QByteArray::fromHex(" + var + ".toString().toLatin1())";
    } else if (type.nameSpace() == XMLSchemaURI && type.localName() == "base64Binary") {
        return "QByteArray::fromBase64(" + var + ".toString().toLatin1())";
    } else if (type.nameSpace() == XMLSchemaURI && type.localName() == "dateTime") {
        Q_ASSERT(qtTypeName == QLatin1String("KDDateTime"));
        return "KDDateTime::fromDateString(" + var + ".toString())";
    } else if (type.nameSpace() == XMLSchemaURI && type.localName() == "anySimpleType") {
        return var;
    } else {
        return var + ".value<" + qtTypeName + ">()";
    }
}

QString KWSDL::TypeMap::serializeBuiltin( const QName &typeName, const QName& elementName, const QString& var, const QString& qtTypeName ) const
{
    Q_UNUSED(qtTypeName);
    const QName type = typeName.isEmpty() ? typeForElement(elementName) : typeName;
    // variantToTextValue also has support for calling toHex/toBase64 at runtime, but this fails
    // when the type derives from hexBinary and is named differently, see Telegram testcase.
    if (type.nameSpace() == XMLSchemaURI && type.localName() == "hexBinary") {
        return "QString::fromLatin1(" + var + ".toHex().constData())";
    } else if (type.nameSpace() == XMLSchemaURI && type.localName() == "base64Binary") {
        return "QString::fromLatin1(" + var + ".toBase64().constData())";
    } else if (type.nameSpace() == XMLSchemaURI && type.localName() == "dateTime") {
        return var + ".toDateString()";
    } else if (type.nameSpace() == XMLSchemaURI && type.localName() == "anySimpleType") {
        return var;
    } else {
        return "QVariant::fromValue(" + var + ")";
    }
}

