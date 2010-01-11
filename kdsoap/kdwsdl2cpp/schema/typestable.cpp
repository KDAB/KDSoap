/* 
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
                       based on wsdlpull parser by Vivek Krishna

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
 
#include "typestable.h"

const QString SchemaUri = "http://www.w3.org/2001/XMLSchema";

namespace XSD {

class TypesTables::Private
{
public:
    Private()
    {
       mCurrentId = XSDType::ANYURI + 1;

       //map of simple types
       mBasicTypes["string"] = XSDType::STRING;
       mBasicTypes["integer"] = XSDType::INTEGER;
       mBasicTypes["int"] = XSDType::INT;
       mBasicTypes["byte"] = XSDType::BYTE;
       mBasicTypes["unsignedByte"] = XSDType::UBYTE;
       mBasicTypes["positiveInteger"] = XSDType::POSINT;
       mBasicTypes["unsignedInt"] = XSDType::UINT;
       mBasicTypes["long"] = XSDType::LONG;
       mBasicTypes["unsignedLong"] = XSDType::ULONG;
       mBasicTypes["short"] = XSDType::SHORT;
       mBasicTypes["unsignedShort"] = XSDType::USHORT;
       mBasicTypes["decimal"] = XSDType::DECIMAL;
       mBasicTypes["float"] = XSDType::FLOAT;
       mBasicTypes["double"] = XSDType::DOUBLE;
       mBasicTypes["boolean"] = XSDType::BOOLEAN;
       mBasicTypes["time"] = XSDType::TIME;
       mBasicTypes["dateTime"] = XSDType::DATETIME;
       mBasicTypes["date"] = XSDType::DATE;
       mBasicTypes["token"] = XSDType::TOKEN;
       mBasicTypes["QName"] = XSDType::QNAME;
       mBasicTypes["NCName"] = XSDType::NCNAME;
       mBasicTypes["NMTOKEN"] = XSDType::NMTOKEN;
       mBasicTypes["NMTOKENS"] = XSDType::NMTOKENS;
       mBasicTypes["base64Binary"] = XSDType::BASE64BIN;
       mBasicTypes["hexBinary"] = XSDType::HEXBIN;
       mBasicTypes["anyType"] = XSDType::ANYTYPE;
       mBasicTypes["any"] = XSDType::ANY;
       mBasicTypes["anyURI"] = XSDType::ANYURI;
    }

    XSDType::List mTypes;

    //maintains a map of all user defined type names and their ids
    QMap<QString, int> mUserTypes;
    QMap<QString, int> mBasicTypes;

    int mCurrentId;

    QString mNameSpace;

    struct ExternRef
    {
      int localTypeId;
      QName qname;
    };

    QList<struct ExternRef> mExternRefs;
}

TypesTable::TypesTable()
  : d(new Private)
{
}

TypesTable::TypesTable( const TypesTable &other )
  : d(new Private)
{
  *d = *other.d;
}

TypesTable::~TypesTable()
{
  clear();
  delete d;
}

TypesTable &TypesTable::operator=( const TypesTable &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void TypesTable::clear()
{
  QMap<QString, int>::Iterator it;
  for ( it = d->mUserTypes.begin(); it != d->mUserTypes.end(); ++it )
    delete typePtr( it.value() );

  d->mUserTypes.clear();
  d->mTypes.clear();
}

int TypesTable::numExtRefs() const
{
  return d->mExternRefs.count();
}

QName TypesTable::extRefName( int index ) const
{
  return d->mExternRefs[ index ].qname;
}

int TypesTable::extRefType( int index ) const
{
  return d->mExternRefs[ index ].localTypeId;
}

int TypesTable::addType( XSDType *type )
{
  QName qn = type->qualifiedName();
  QString type_name( qn.localName() );

  int i = 0;

  if ( type_name.isEmpty() ) {
    // create an anonymous type name
    type_name.sprintf( "type%d", d->mTypes.count() );
    type->setName( type_name );
    type->setAnonymous( true );  //auto generated name
  }

  // add the typename and its id  to the map
  if ( (i = d->mUserTypes[type_name]) != 0 ) {
    // this type was refernced earlier.
    d->mTypes[i - (XSDType::ANYURI + 1)] = type;
    type->setType( i );
    return i;
  } else {
    d->mUserTypes[ type_name ] = mCurrentId;
    type->setType( mCurrentId );
    d->mTypes.append( type );
    d->mCurrentId++;
    return d->mCurrentId - 1;
  }
}

int TypesTable::typeId( const QName &name, bool create )
{
  int typeId;

  if ( name.nameSpace() == SchemaUri ) { // this is one of the basic types
    typeId = d->mBasicTypes[ name.localName() ];
    if ( typeId == 0 ) // if this is a basic type which is not mapped, treat as string
      typeId = XSDType::STRING;
  } else if ( name.nameSpace() == d->mNameSpace )
    typeId = d->mUserTypes[ name.localName() ];
  else { // the type does not belong to this schema
    return 0;
  }

  if ( typeId == 0 && create ) {
    // handle forward reference
    // create an id and return its value
    d->mUserTypes[name.localName()] = d->mCurrentId;
    d->mTypes.append( 0 );
    d->mCurrentId++;
    typeId = d->mCurrentId - 1;
  }

  return typeId;
}

QString TypesTable::typeName( int id ) const
{
  if ( id < 0 )
    return QString();

  QMap<QString, int>::ConstIterator it;

  if ( id >= 0 && id <= XSDType::ANYURI ) {
    for ( it = d->mBasicTypes.begin(); it != d->mBasicTypes.end(); ++it )
      if ( id == it.value() )
        return it.key();
  }

  for ( it = d->mUserTypes.begin(); it != d->mUserTypes.end(); ++it )
    if ( id == it.value() )
      return it.key();

  return "<unknown type>";
}

int TypesTable::addExternalTypeId( const QName &type, XSDType *pType )
{
  for ( int i = 0; i < (int)d->mExternRefs.count(); i++ )
    if ( d->mExternRefs[i].qname == type )
      return d->mExternRefs[i].localTypeId;

  struct ExternRef ref;
  ref.qname = (pType) ? pType->qualifiedName() : type;
  ref.localTypeId = d->mCurrentId;
  d->mExternRefs.append( ref );

  d->mTypes.append( pType );
  d->mCurrentId++;

  return ref.localTypeId;
}

// adds a type into a type table for a given type id
// used for types present in imported schemas but referenced in current schema
int TypesTable::addExtType( XSDType *type, int localId )
{
  int index = localId - XSDType::ANYURI - 1;
  if ( index >= (int)d->mTypes.count() )
    return 0;

  d->mTypes[ index ] = type;
  return localId;
}

bool TypesTable::detectUndefinedTypes()
{
  for ( int i = 0; i < (int)d->mTypes.count(); i++ )
    if ( d->mTypes[i] == 0 )
      return true;

  return false;
}

void TypesTable::resolveForwardElementRefs( const QString &name, Element &element )
{
  for ( int i = 0; i < (int)d->mTypes.count(); i++ )
    if ( d->mTypes[i] != 0 ) {
      if ( !d->mTypes[i]->isSimple() ) {
        ComplexType *ct = (ComplexType*)d->mTypes[i];
        ct->matchElementRef( name, element );
      }
    }
}

void TypesTable::resolveForwardAttributeRefs( const QString &name, Attribute &attribute )
{
  for ( int i = 0; i < (int)d->mTypes.count(); i++ )
    if ( d->mTypes[i] != 0 ) {
      if ( !d->mTypes[i]->isSimple() ) {
        ComplexType *ct = (ComplexType*)d->mTypes[i];
        ct->matchAttributeRef( name, attribute );
      }
    }
}

XSDType *TypesTable::typePtr( int id ) const
{
  // this is a basic XSD type
  if ( id < XSDType::ANYURI + 1 || id > XSDType::ANYURI + (int)d->mTypes.count() )
    return 0;

  return d->mTypes[ id - (SimpleType::ANYURI + 1) ];
}

int TypesTable::numTypes() const
{
  return d->mTypes.count();
}

void TypesTable::setTargetNamespace( const QString &nameSpace )
{
  d->mNameSpace = nameSpace;
}

QString TypesTable::targetNamespace() const
{
  return d->mNameSpace;
}

}
