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

namespace XSD
{

class Types::Private
{
public:
    SimpleType::List mSimpleTypes;
    ComplexType::List mComplexTypes;
    Element::List mElements;
    Attribute::List mAttributes;
#if 0
    AttributeGroup::List mAttributeGroups;
    Group::List mGroups;
#endif
};

Types::Types()
    : d(new Private)
{
}

Types::Types(const Types &other)
    : d(new Private)
{
    *d = *other.d;
}

Types::~Types()
{
    delete d;
}

Types &Types::operator=(const Types &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;

    return *this;
}

Types &Types::operator +=(const Types &other)
{
    if (this == &other) {
        return *this;
    }

    d->mSimpleTypes += other.d->mSimpleTypes;
    d->mComplexTypes += other.d->mComplexTypes;
    d->mElements += other.d->mElements;
    d->mAttributes += other.d->mAttributes;
    // unused d->mAttributeGroups += other.d->mAttributeGroups;
    // unused d->mGroups += other.d->mGroups;

    return *this;
}

void Types::setSimpleTypes(const SimpleType::List &simpleTypes)
{
    d->mSimpleTypes = simpleTypes;
}

SimpleType::List Types::simpleTypes() const
{
    return d->mSimpleTypes;
}

void Types::setComplexTypes(const ComplexType::List &complexTypes)
{
    d->mComplexTypes = complexTypes;
}

ComplexType::List Types::complexTypes() const
{
    return d->mComplexTypes;
}

void Types::setElements(const Element::List &elements)
{
    d->mElements = elements;
}

Element::List Types::elements() const
{
    return d->mElements;
}

void Types::setAttributes(const Attribute::List &attributes)
{
    d->mAttributes = attributes;
}

Attribute::List Types::attributes() const
{
    return d->mAttributes;
}

#if 0
void Types::setAttributeGroups(const AttributeGroup::List &attributeGroups)
{
    d->mAttributeGroups = attributeGroups;
}

AttributeGroup::List Types::attributeGroups() const
{
    return d->mAttributeGroups;
}

void Types::setGroups(const Group::List &groups)
{
    d->mGroups = groups;
}

Group::List Types::groups() const
{
    return d->mGroups;
}

ComplexType Types::complexType(const Element &element) const
{
    return complexType(element.type());
}
#endif

ComplexType Types::complexType(const QName &typeName) const
{
    return d->mComplexTypes.complexType(typeName);
}

ComplexType Types::polymorphicBaseClass(const ComplexType &derivedType) const
{
    if (derivedType.isPolymorphicBaseClass()) {
        return derivedType;
    }
    ComplexType base = complexType(derivedType.baseTypeName());
    if (!base.isNull()) {
        return polymorphicBaseClass(base);    // recurse
    }
    return ComplexType();
}

SimpleType Types::simpleType(const QName &typeName) const
{
    return d->mSimpleTypes.simpleType(typeName);
}

} // namespace XSD
