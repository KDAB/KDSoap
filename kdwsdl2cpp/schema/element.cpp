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

#include "element.h"
#include <QDebug>

namespace XSD
{

class Element::Private
{
public:
    Private()
        : mGroupId(0),
          mMinOccurs(1),
          mMaxOccurs(1),
          mQualified(false),
          mNillable(false),
          mHasSubstitutions(false),
          mOccurrence(0)
    {}

    QName mType;
    QString mDocumentation;
    int mGroupId;
    int mMinOccurs;
    int mMaxOccurs;
    bool mQualified;
    bool mNillable;
    bool mHasSubstitutions;
    QString mDefaultValue;
    QString mFixedValue;
    int mOccurrence;
    QName mReference;
    Compositor mCompositor;
};

Element::Element()
    : XmlElement(), d(new Private)
{
}

Element::Element(const QString &nameSpace)
    : XmlElement(nameSpace), d(new Private)
{
}

Element::Element(const Element &other)
    : XmlElement(other), d(new Private)
{
    *d = *other.d;
}

Element::~Element()
{
    delete d;
}

Element &Element::operator=(const Element &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;
    XmlElement::operator=(other);

    return *this;
}

void Element::setType(const QName &type)
{
    Q_ASSERT(!type.isEmpty());
    d->mType = type;
}

QName Element::type() const
{
    return d->mType;
}

void Element::setDocumentation(const QString &documentation)
{
    d->mDocumentation = documentation;
}

QString Element::documentation() const
{
    return d->mDocumentation;
}

// unused
void Element::setGroupId(int group)
{
    d->mGroupId = group;
}

// unused
int Element::groupId() const
{
    return d->mGroupId;
}

void Element::setMinOccurs(int minOccurs)
{
    d->mMinOccurs = minOccurs;
}

int Element::minOccurs() const
{
    return d->mMinOccurs;
}

void Element::setMaxOccurs(int maxOccurs)
{
    d->mMaxOccurs = maxOccurs;
}

int Element::maxOccurs() const
{
    return d->mMaxOccurs;
}

void Element::setDefaultValue(const QString &defaultValue)
{
    d->mDefaultValue = defaultValue;
}

QString Element::defaultValue() const
{
    return d->mDefaultValue;
}

void Element::setFixedValue(const QString &fixedValue)
{
    d->mFixedValue = fixedValue;
}

QString Element::fixedValue() const
{
    return d->mFixedValue;
}

void Element::setIsQualified(bool isQualified)
{
    d->mQualified = isQualified;
}

bool Element::isQualified() const
{
    return d->mQualified;
}

void Element::setNillable(bool nillable)
{
    d->mNillable = nillable;
}

bool Element::nillable() const
{
    return d->mNillable;
}

void Element::setOccurrence(int occurrence)
{
    d->mOccurrence = occurrence;
}

int Element::occurrence() const
{
    return d->mOccurrence;
}

void Element::setReference(const QName &reference)
{
    Q_ASSERT(!reference.isEmpty());
    d->mReference = reference;
}

QName Element::reference() const
{
    return d->mReference;
}

bool Element::isResolved() const
{
    return !d->mType.isEmpty() || d->mReference.isEmpty();
}

void Element::setCompositor(const Compositor &c)
{
    d->mCompositor = c;
}

Compositor Element::compositor() const
{
    return d->mCompositor;
}

void Element::setHasSubstitutions(bool hasSub)
{
    d->mHasSubstitutions = hasSub;
}

bool Element::hasSubstitutions() const
{
    return d->mHasSubstitutions;
}

Element ElementList::element(const QName &qualifiedName) const
{
    const_iterator it = constBegin();
    for (; it != constEnd(); ++it)
        if ((*it).qualifiedName() == qualifiedName) {
            return *it;
        }
    //qDebug() << "Simple type" << qualifiedName << "not found";
    return Element();
}

ElementList::iterator ElementList::findElement(const QName &qualifiedName)
{
    for (iterator it = begin(); it != end(); ++it)
        if ((*it).qualifiedName() == qualifiedName) {
            return it;
        }
    return end();

}

void ElementList::dump()
{
    Q_FOREACH (const Element &element, *this) {
        qDebug() << element.nameSpace() << element.name();
    }
}

} // namespace XSD

QDebug operator<<(QDebug dbg, const XSD::Element &element)
{
    dbg << element.name();
    return dbg;
}
