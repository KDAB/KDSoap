/****************************************************************************
** Copyright (C) 2010-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

/*
    This file is part of KDE Schema Parser.

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef ATTRIBUTEGROUP_H
#define ATTRIBUTEGROUP_H

#include <schema/xmlelement.h>
#include <schema/attribute.h>
#include <common/qname.h>

#include <kode_export.h>

namespace XSD {

class SCHEMA_EXPORT AttributeGroup : public XmlElement
{
  public:
    typedef QList<AttributeGroup> List;

    AttributeGroup();
    AttributeGroup( const AttributeGroup &other );
    ~AttributeGroup();

    AttributeGroup &operator=( const AttributeGroup &other );

    void setReference( const QName &reference );
    QName reference() const;

    void setAttributes( const Attribute::List &attributes );
    Attribute::List attributes() const;

    bool isResolved() const;

  private:
    class Private;
    Private *d;
};

}

#endif
