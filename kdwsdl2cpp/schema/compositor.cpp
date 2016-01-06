/*
    This file is part of KDE Schema Parser

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

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

#include "compositor.h"

namespace XSD
{

class Compositor::Private
{
public:
    Private()
        : mType(Invalid),
          mMinOccurs(1),
          mMaxOccurs(1)
    {}

    Type mType;
    QName::List mChildren;
    int mMinOccurs;
    int mMaxOccurs;
};

Compositor::Compositor()
    : d(new Private)
{
}

Compositor::Compositor(Type type)
    : d(new Private)
{
    d->mType = type;
}

Compositor::Compositor(const Compositor &other)
    : d(new Private)
{
    *d = *other.d;
}

Compositor::~Compositor()
{
    delete d;
}

Compositor &Compositor::operator=(const Compositor &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;

    return *this;
}

bool Compositor::isValid() const
{
    return d->mType != Invalid;
}

void Compositor::setMinOccurs(int minOccurs)
{
    d->mMinOccurs = minOccurs;
}

int Compositor::minOccurs() const
{
    return d->mMinOccurs;
}

void Compositor::setMaxOccurs(int maxOccurs)
{
    d->mMaxOccurs = maxOccurs;
}

int Compositor::maxOccurs() const
{
    return d->mMaxOccurs;
}

void Compositor::setType(Type type)
{
    d->mType = type;
}

Compositor::Type Compositor::type() const
{
    return d->mType;
}

void Compositor::addChild(const QName &child)
{
    d->mChildren.append(child);
}

void Compositor::setChildren(const QName::List &children)
{
    d->mChildren = children;
}

QName::List Compositor::children() const
{
    return d->mChildren;
}

}
