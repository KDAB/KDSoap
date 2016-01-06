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

#ifndef SCHEMA_COMPLEXTYPE_H
#define SCHEMA_COMPLEXTYPE_H

#include <QString>

#include <schema/attribute.h>
#include <schema/attributegroup.h>
#include <schema/group.h>
#include <schema/element.h>
#include <schema/xsdtype.h>

#include <common/qname.h>
#include <kode_export.h>

namespace XSD
{
class ComplexTypeList;

class SCHEMA_EXPORT ComplexType : public XSDType
{
public:
    typedef ComplexTypeList List;

    typedef enum {
        Restriction,
        Extension
    } Derivation;

    ComplexType();
    ComplexType(const QString &nameSpace);
    ComplexType(const ComplexType &other);
    ~ComplexType();

    ComplexType &operator=(const ComplexType &other);

    void setDocumentation(const QString &documentation);
    QString documentation() const;

    bool isSimple() const;

    // True if this is the base class for other (derived) complex types
    bool isPolymorphicBaseClass() const;

    void setAnonymous(bool anonymous);
    bool isAnonymous() const;

    void setConflicting(bool c);
    bool isConflicting() const;

    void setBaseDerivation(Derivation derivation);
    Derivation baseDerivation() const;

    void setBaseTypeName(const QName &baseTypeName);
    QName baseTypeName() const;

    void addDerivedType(const QName &derivedTypeName);
    QList<QName> derivedTypes() const;

    void setElements(const Element::List &elements);
    Element::List elements() const;

    void setGroups(const Group::List &groups);
    void addGroup(const Group &group);
    Group::List groups() const;

    void setAttributes(const Attribute::List &attributes);
    Attribute::List attributes() const;
    Attribute attribute(const QName &attrName) const;

    void addAttributeGroups(const AttributeGroup &attributeGroups);
    void setAttributeGroups(const AttributeGroup::List &attributeGroups);
    AttributeGroup::List attributeGroups() const;

    void setArrayType(const QName &arrayType);
    QName arrayType() const;
    bool isArray() const;

    void addAttribute(const Attribute &attribute);
    void addElement(const Element &element);

    bool isEmpty() const;

private:
    class Private;
    Private *d;
};

class SCHEMA_EXPORT ComplexTypeList : public QList<ComplexType>
{
public:
    ComplexTypeList(const QList<ComplexType> &arg) : QList<ComplexType>(arg) {}
    ComplexTypeList() : QList<ComplexType>() {}

    // Readonly lookup, returns null type if not found
    ComplexType complexType(const QName &qualifiedName) const;

    // Mutable lookup (for making changes), returns end() if not found
    iterator findComplexType(const QName &qualifiedName);
};

} // namespace XSD

SCHEMA_EXPORT QDebug operator<<(QDebug dbg, const XSD::ComplexType &type);

#endif
