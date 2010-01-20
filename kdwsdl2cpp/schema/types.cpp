/* 
    This file is part of KDE Schema Parser

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

#include "types.h"

#include <QDebug>

namespace XSD {

class Types::Private
{
public:
    SimpleType::List mSimpleTypes;
    ComplexType::List mComplexTypes;
    Element::List mElements;
    Attribute::List mAttributes;
    AttributeGroup::List mAttributeGroups;
    QStringList mNamespaces;
};

Types::Types()
  : d(new Private)
{
}

Types::Types( const Types &other )
  : d(new Private)
{
  *d = *other.d;
}

Types::~Types()
{
  delete d;
}

Types &Types::operator=( const Types &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void Types::setSimpleTypes( const SimpleType::List &simpleTypes )
{
  d->mSimpleTypes = simpleTypes;
}

SimpleType::List Types::simpleTypes() const
{
  return d->mSimpleTypes;
}

void Types::setComplexTypes( const ComplexType::List &complexTypes )
{
  d->mComplexTypes = complexTypes;
}

ComplexType::List Types::complexTypes() const
{
  return d->mComplexTypes;
}

void Types::setElements( const Element::List &elements )
{
  d->mElements = elements;
}

Element::List Types::elements() const
{
  return d->mElements;
}

void Types::setAttributes( const Attribute::List &attributes )
{
  d->mAttributes = attributes;
}

Attribute::List Types::attributes() const
{
  return d->mAttributes;
}

void Types::setAttributeGroups( const AttributeGroup::List &attributeGroups )
{
  d->mAttributeGroups = attributeGroups;
}

AttributeGroup::List Types::attributeGroups() const
{
  return d->mAttributeGroups;
}

void Types::setNamespaces( const QStringList &namespaces )
{
  d->mNamespaces = namespaces;
}

QStringList Types::namespaces() const
{
  return d->mNamespaces;
}

#if 0
ComplexType Types::complexType( const Element &element ) const
{
  return complexType( element.type() );
}
#endif

ComplexType Types::complexType( const QName &typeName ) const
{
  foreach( const ComplexType& type, d->mComplexTypes ) {
    if( typeName == type.qualifiedName() ) return type;
  }
  return ComplexType();
}

SimpleType Types::simpleType( const QName &typeName ) const
{
  foreach( const SimpleType& type, d->mSimpleTypes ) {
    if ( type.qualifiedName() == typeName ) return type;
  }
  qDebug() << "Types::simpleType():" << typeName << "not found";
  return SimpleType();
}

}
