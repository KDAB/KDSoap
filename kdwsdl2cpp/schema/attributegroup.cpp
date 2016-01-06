/*
    This file is part of KDE Schema Parser.

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "attributegroup.h"

namespace XSD
{

class AttributeGroup::Private
{
public:
    QName mReference;
    Attribute::List mAttributes;
};

AttributeGroup::AttributeGroup()
    : XmlElement(), d(new Private)
{
}

AttributeGroup::AttributeGroup(const AttributeGroup &other)
    : XmlElement(other), d(new Private)
{
    *d = *other.d;
}

AttributeGroup::~AttributeGroup()
{
    delete d;
}

AttributeGroup &AttributeGroup::operator=(const AttributeGroup &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;

    return *this;
}

void AttributeGroup::setReference(const QName &reference)
{
    d->mReference = reference;
}

QName AttributeGroup::reference() const
{
    return d->mReference;
}

void AttributeGroup::setAttributes(const Attribute::List &attributes)
{
    d->mAttributes = attributes;
}

Attribute::List AttributeGroup::attributes() const
{
    return d->mAttributes;
}

}
