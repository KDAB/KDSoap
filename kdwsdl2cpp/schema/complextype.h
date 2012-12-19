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
#include <schema/element.h>
#include <schema/xsdtype.h>

#include <common/qname.h>
#include <kode_export.h>

namespace XSD {

class SCHEMA_EXPORT ComplexType : public XSDType
{
  public:
    typedef QList<ComplexType> List;

    typedef enum {
      Restriction,
      Extension
    } Derivation;

    ComplexType();
    ComplexType( const QString &nameSpace );
    ComplexType( const ComplexType &other );
    ~ComplexType();

    ComplexType &operator=( const ComplexType &other );

    void setDocumentation( const QString &documentation );
    QString documentation() const;

    bool isSimple() const;

    void setAnonymous( bool anonymous );
    bool isAnonymous() const;

    void setConflicting( bool c );
    bool isConflicting() const;

    void setBaseDerivation( Derivation derivation );
    Derivation baseDerivation() const;

    void setBaseTypeName( const QName &baseTypeName );
    QName baseTypeName() const;

    void setElements( const Element::List &elements );
    Element::List elements() const;

    void setAttributes( const Attribute::List &attributes );
    Attribute::List attributes() const;
    Attribute attribute( const QName& attrName ) const;

    void setAttributeGroups( const AttributeGroup::List &attributeGroups );
    AttributeGroup::List attributeGroups() const;

    void setArrayType( const QName &arrayType );
    QName arrayType() const;
    bool isArray() const;

    void addAttribute( const Attribute &attribute );
    void addElement( const Element &element );

    bool isEmpty() const;

  private:
    class Private;
    Private *d;
};

}

#endif
