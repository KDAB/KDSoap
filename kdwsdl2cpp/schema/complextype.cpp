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
#include <QDebug>

namespace XSD
{

class ComplexType::Private
{
public:
    Private()
        : mAnonymous(false),
          mConflicting(false),
          mBaseDerivation(Restriction)
    {}

    QString mDocumentation;

    Element::List mElements;
    Attribute::List mAttributes;
    Group::List mGroups;
    AttributeGroup::List mAttributeGroups;

    bool mAnonymous;
    bool mConflicting;

    Derivation mBaseDerivation;
    QName mBaseTypeName;
    QName mArrayType;
    QList<QName> mDerivedTypes;
};

ComplexType::ComplexType(const QString &nameSpace)
    : XSDType(nameSpace), d(new Private)
{
}

ComplexType::ComplexType()
    : XSDType(), d(new Private)
{
}

ComplexType::ComplexType(const ComplexType &other)
    : XSDType(other), d(new Private)
{
    *d = *other.d;
}

ComplexType::~ComplexType()
{
    delete d;
}

ComplexType &ComplexType::operator=(const ComplexType &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;

    return *this;
}

void ComplexType::setDocumentation(const QString &documentation)
{
    d->mDocumentation = documentation;
}

QString ComplexType::documentation() const
{
    return d->mDocumentation;
}

void ComplexType::setBaseTypeName(const QName &baseTypeName)
{
    d->mBaseTypeName = baseTypeName;
}

QName ComplexType::baseTypeName() const
{
    return d->mBaseTypeName;
}

void ComplexType::addDerivedType(const QName &derivedTypeName)
{
    d->mDerivedTypes.append(derivedTypeName);
}

QList<QName> ComplexType::derivedTypes() const
{
    return d->mDerivedTypes;
}

void ComplexType::setBaseDerivation(Derivation derivation)
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

bool ComplexType::isPolymorphicBaseClass() const
{
    return !isArray() && !d->mDerivedTypes.isEmpty() && d->mBaseTypeName.isEmpty();
}

void ComplexType::setArrayType(const QName &arrayType)
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

void ComplexType::setAnonymous(bool anonymous)
{
    d->mAnonymous = anonymous;
}

bool ComplexType::isAnonymous() const
{
    return d->mAnonymous;
}

void ComplexType::setConflicting(bool conflicting)
{
    d->mConflicting = conflicting;
}

bool ComplexType::isConflicting() const
{
    return d->mConflicting;
}

void ComplexType::setElements(const Element::List &elements)
{
    d->mElements = elements;
}

Element::List ComplexType::elements() const
{
    return d->mElements;
}

void ComplexType::setGroups(const Group::List &groups)
{
    d->mGroups = groups;
}

void ComplexType::addGroup(const Group &group)
{
    d->mGroups.append(group);
}

Group::List ComplexType::groups() const
{
    return d->mGroups;
}

void ComplexType::setAttributes(const Attribute::List &attributes)
{
    d->mAttributes = attributes;
}

Attribute::List ComplexType::attributes() const
{
    return d->mAttributes;
}

void ComplexType::addAttributeGroups(const AttributeGroup &attributeGroups)
{
    d->mAttributeGroups.append(attributeGroups);
}

void ComplexType::setAttributeGroups(const AttributeGroup::List &attributeGroups)
{
    d->mAttributeGroups = attributeGroups;
}

AttributeGroup::List ComplexType::attributeGroups() const
{
    return d->mAttributeGroups;
}

void ComplexType::addAttribute(const Attribute &attribute)
{
    d->mAttributes.append(attribute);
}

Attribute ComplexType::attribute(const QName &attrName) const
{
    Q_FOREACH (const Attribute &attr, d->mAttributes) {
        if (attr.qualifiedName() == attrName) {
            return attr;
        }
    }
    return Attribute();
}

void ComplexType::addElement(const Element &element)
{
    d->mElements.append(element);
}

bool ComplexType::isEmpty() const
{
    return d->mAttributeGroups.isEmpty() && d->mGroups.isEmpty() && d->mAttributes.isEmpty() && d->mElements.isEmpty() && d->mBaseTypeName.isEmpty() && d->mArrayType.isEmpty();
}

ComplexType ComplexTypeList::complexType(const QName &qualifiedName) const
{
    //qDebug() << "looking for" << typeName << "ns=" << typeName.nameSpace();
    foreach (const ComplexType &type, *this) {
        //qDebug() << type.nameSpace() << "qualifiedName=" << type.qualifiedName();
        if (qualifiedName == type.qualifiedName()) {
            return type;
        }
    }
    //qDebug() << "Complex type" << qualifiedName << "not found";
    return ComplexType();
}

ComplexTypeList::iterator ComplexTypeList::findComplexType(const QName &qualifiedName)
{
    for (iterator it = begin(); it != end(); ++it)
        if ((*it).qualifiedName() == qualifiedName) {
            return it;
        }
    return end();
}

} // namespace XSD

QDebug operator<<(QDebug dbg, const XSD::ComplexType &type)
{
    dbg << type.qualifiedName();
    // dbg << type.attributes();
    return dbg;
}
