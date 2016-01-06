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
#ifndef ATTRIBUTEGROUP_H
#define ATTRIBUTEGROUP_H

#include <schema/xmlelement.h>
#include <schema/attribute.h>
#include <common/qname.h>

#include <kode_export.h>

namespace XSD
{

class SCHEMA_EXPORT AttributeGroup : public XmlElement
{
public:
    typedef QList<AttributeGroup> List;

    AttributeGroup();
    AttributeGroup(const AttributeGroup &other);
    ~AttributeGroup();

    AttributeGroup &operator=(const AttributeGroup &other);

    void setReference(const QName &reference);
    QName reference() const;

    void setAttributes(const Attribute::List &attributes);
    Attribute::List attributes() const;

private:
    class Private;
    Private *d;
};

}

#endif
