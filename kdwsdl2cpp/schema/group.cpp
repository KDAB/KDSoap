/*
    This file is part of KDE Schema Parser.

    Copyright (c) 2013 David Faure <david.faure@kdab.com>

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

#include "group.h"

namespace XSD
{

class Group::Private
{
public:
    QName mReference;
    Element::List mElements;
};

Group::Group()
    : XmlElement(), d(new Private)
{
}

Group::Group(const Group &other)
    : XmlElement(other), d(new Private)
{
    *d = *other.d;
}

Group::~Group()
{
    delete d;
}

Group &Group::operator=(const Group &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;

    return *this;
}

void Group::setReference(const QName &reference)
{
    d->mReference = reference;
}

QName Group::reference() const
{
    return d->mReference;
}

void Group::setElements(const Element::List &elements)
{
    d->mElements = elements;
}

Element::List Group::elements() const
{
    return d->mElements;
}

bool Group::isResolved() const
{
    return !d->mElements.isEmpty() || d->mReference.isEmpty();
}

}

QDebug operator<<(QDebug dbg, const XSD::Group &group)
{
    dbg << group.qualifiedName();
    return dbg;
}
