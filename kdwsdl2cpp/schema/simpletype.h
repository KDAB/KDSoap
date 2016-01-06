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

#ifndef SCHEMA_SIMPLETYPE_H
#define SCHEMA_SIMPLETYPE_H

#include <QStringList>

#include <common/qname.h>
#include "xsdtype.h"
#include <kode_export.h>

namespace XSD
{

class SimpleTypeList;

class SCHEMA_EXPORT SimpleType : public XSDType
{
public:
    typedef SimpleTypeList List;

    enum FacetType {
        NONE = 0,
        LENGTH = 1,
        MINLEN = 2,
        MAXLEN = 4,
        ENUM = 8,
        WSP = 16,
        MAXINC = 32,
        MININC = 64,
        MAXEX = 128,
        MINEX = 256,
        TOT = 512,
        FRAC = 1024,
        PATTERN = 2048
    };

    enum WhiteSpaceType {
        PRESERVE,
        REPLACE,
        COLLAPSE
    };

    enum SubType {
        TypeRestriction,
        TypeList,
        TypeUnion
    };

    SimpleType();
    SimpleType(const QString &nameSpace);
    SimpleType(const SimpleType &other);
    ~SimpleType();

    SimpleType &operator=(const SimpleType &other);

    void setDocumentation(const QString &documentation);
    QString documentation() const;

    void setBaseTypeName(const QName &baseTypeName);
    QName baseTypeName() const;

    void setSubType(SubType subType);
    SubType subType() const;

    void setListTypeName(const QName &name);
    QName listTypeName() const;

    void setAnonymous(bool anonymous);
    bool isAnonymous() const;

    FacetType parseFacetId(const QString &facet) const;
    void setFacetValue(FacetType ft, const QString &value);

    int facetType() const;

    int facetLength() const;
    int facetMinimumLength() const;
    int facetMaximumLength() const;
    QStringList facetEnums() const;
    WhiteSpaceType facetWhiteSpace() const;
    int facetMinimumInclusive() const;
    int facetMaximumInclusive() const;
    int facetMinimumExclusive() const;
    int facetMaximumExclusive() const;
    int facetTotalDigits() const;
    int facetFractionDigits() const;
    QString facetPattern() const;

    /**
     * Return true if this type is just a restriction of another type.
     * False for enums, and false if the base type is any.
     */
    bool isRestriction() const;

private:
    class Private;
    Private *d;
};

class SCHEMA_EXPORT SimpleTypeList : public QList<SimpleType>
{
public:
    // Readonly lookup, returns null type if not found
    SimpleType simpleType(const QName &qualifiedName) const;

    // Mutable lookup (for making changes), returns end() if not found
    iterator findSimpleType(const QName &qualifiedName);
};

}

#endif
