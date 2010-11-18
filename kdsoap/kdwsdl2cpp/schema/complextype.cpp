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

#include "complextype.h"

namespace XSD {

class ComplexType::Private
{
public:
    Private()
      : mAnonymous(false), mIsArray(false)
    {}

    QString mDocumentation;

    Element::List mElements;
    Attribute::List mAttributes;
    AttributeGroup::List mAttributeGroups;

    bool mAnonymous;
    bool mIsArray;

    Derivation mBaseDerivation;
    QName mBaseTypeName;
    QName mArrayType;
};

ComplexType::ComplexType( const QString &nameSpace )
  : XSDType( nameSpace ), d(new Private)
{
}

ComplexType::ComplexType()
  : XSDType(), d(new Private)
{
}

ComplexType::ComplexType( const ComplexType &other )
  : XSDType( other ), d(new Private)
{
  *d = *other.d;
}

ComplexType::~ComplexType()
{
  delete d;
}

ComplexType &ComplexType::operator=( const ComplexType &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void ComplexType::setDocumentation( const QString &documentation )
{
  d->mDocumentation = documentation;
}

QString ComplexType::documentation() const
{
  return d->mDocumentation;
}

void ComplexType::setBaseTypeName( const QName &baseTypeName )
{
  d->mBaseTypeName = baseTypeName;
}

QName ComplexType::baseTypeName() const
{
  return d->mBaseTypeName;
}

void ComplexType::setBaseDerivation( Derivation derivation )
{
  d->mBaseDerivation = derivation;
}

ComplexType::Derivation ComplexType::baseDerivation() const
{
  return d->mBaseDerivation;
}

bool ComplexType::isSimple() const
{
  return false;
}

void ComplexType::setArrayType( const QName &arrayType )
{
  d->mArrayType = arrayType;
}

QName ComplexType::arrayType() const
{
  return d->mArrayType;
}

bool ComplexType::isArray() const
{
    return !arrayType().isEmpty();
}

void ComplexType::setAnonymous( bool anonymous )
{
  d->mAnonymous = anonymous;
}

bool ComplexType::isAnonymous() const
{
  return d->mAnonymous;
}

void ComplexType::setElements( const Element::List &elements )
{
  d->mElements = elements;
}

Element::List ComplexType::elements() const
{
  return d->mElements;
}

void ComplexType::setAttributes( const Attribute::List &attributes )
{
  d->mAttributes = attributes;
}

Attribute::List ComplexType::attributes() const
{
  return d->mAttributes;
}

void ComplexType::setAttributeGroups( const AttributeGroup::List &attributeGroups )
{
  d->mAttributeGroups = attributeGroups;
}

AttributeGroup::List ComplexType::attributeGroups() const
{
  return d->mAttributeGroups;
}

void ComplexType::addAttribute( const Attribute &attribute )
{
  d->mAttributes.append( attribute );
}

Attribute ComplexType::attribute(const QName &attrName) const
{
    Q_FOREACH(const Attribute& attr, d->mAttributes) {
        if (attr.qualifiedName() == attrName)
            return attr;
    }
    return Attribute();
}

void ComplexType::addElement( const Element &element )
{
    QName anyType( "http://www.w3.org/2001/XMLSchema", "any" );
    if (!d->mElements.isEmpty() && d->mElements.last().type() == anyType) {
        // Hack for deserialization: keep "any" last.
        Element lastElem = d->mElements.takeLast();
        d->mElements.append( element );
        d->mElements.append( lastElem );
    } else {
        d->mElements.append( element );
    }
}

bool ComplexType::isEmpty() const
{
  return d->mAttributeGroups.isEmpty() && d->mAttributes.isEmpty() && d->mElements.isEmpty();
}

}
