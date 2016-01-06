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

#include "annotation.h"

#include <common/qname.h>

namespace XSD
{

class Annotation::Private
{
public:
    QDomElement mDomElement;
};

Annotation::Annotation()
    : d(new Private)
{
}

Annotation::Annotation(const QDomElement &element)
    : d(new Private)
{
    d->mDomElement = element;
}

Annotation::Annotation(const Annotation &other)
    : d(new Private)
{
    *d = *other.d;
}

Annotation::~Annotation()
{
    delete d;
}

Annotation &Annotation::operator=(const Annotation &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;

    return *this;
}
void Annotation::setDomElement(const QDomElement &element)
{
    d->mDomElement = element;
}

QDomElement Annotation::domElement() const
{
    return d->mDomElement;
}

bool Annotation::isDocumentation() const
{
    return QName(d->mDomElement.tagName()).localName() == QLatin1String("documentation");
}

bool Annotation::isAppinfo() const
{
    return QName(d->mDomElement.tagName()).localName() == QLatin1String("appinfo");
}

QString Annotation::documentation() const
{
    QString result;

    if (isDocumentation()) {
        result = d->mDomElement.text().trimmed();
    }

    return result;
}

QString Annotation::List::documentation() const
{
    QString result;

    foreach (Annotation a, *this) {
        result.append(a.documentation());
    }

    return result;
}

}
