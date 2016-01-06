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

#ifndef SCHEMA_TYPES_H
#define SCHEMA_TYPES_H

#include "complextype.h"
#include "element.h"
#include "simpletype.h"
#include <kode_export.h>

namespace XSD
{

class SCHEMA_EXPORT Types
{
public:
    Types();
    Types(const Types &other);
    ~Types();

    Types &operator=(const Types &other);
    Types &operator+=(const Types &other);

    void setSimpleTypes(const SimpleType::List &simpleTypes);
    SimpleType::List simpleTypes() const;

    void setComplexTypes(const ComplexType::List &complexTypes);
    ComplexType::List complexTypes() const;

    void setElements(const Element::List &elements);
    Element::List elements() const;

    void setAttributes(const Attribute::List &attributes);
    Attribute::List attributes() const;

    // unused void setAttributeGroups( const AttributeGroup::List &attributeGroups );
    // unused AttributeGroup::List attributeGroups() const;

    // unused void setGroups( const Group::List &groups );
    // unused Group::List groups() const;

    //ComplexType complexType( const Element & ) const;
    ComplexType complexType(const QName &) const;

    // Call this with derived complex type, to find the root of the hierarchy,
    // i.e. the type for which isPolymorphicBaseClass() returns true
    ComplexType polymorphicBaseClass(const ComplexType &derivedType) const;

    SimpleType simpleType(const QName &) const;

private:
    class Private;
    Private *d;
};

}

#endif
