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

#ifndef SCHEMA_XSDTYPE_H
#define SCHEMA_XSDTYPE_H

#include <QList>
#include <QMap>
#include <QString>

#include <common/qname.h>
#include <kode_export.h>

#include "xmlelement.h"

namespace XSD
{

class SCHEMA_EXPORT XSDType : public XmlElement
{
public:
    typedef QList<const XSDType *> List;

    enum ContentModel {
        SIMPLE = 0,
        COMPLEX,
        MIXED
    };
    /*
        enum
        {
          INVALID = 0,
          STRING = 1,
          INTEGER,
          INT,
          BYTE,
          UBYTE,
          POSINT,
          UINT,
          LONG,
          ULONG,
          SHORT,
          USHORT,
          DECIMAL,
          FLOAT,
          DOUBLE,
          BOOLEAN,
          TIME,
          DATETIME,
          DATE,
          TOKEN,
          QNAME,
          NCNAME,
          NMTOKEN,
          NMTOKENS,
          BASE64BIN,
          HEXBIN,
          ANY,
          ANYTYPE,
          ANYURI
        };
    */
    XSDType();
    XSDType(const QString &);
    XSDType(const XSDType &other);
    virtual ~XSDType();

    XSDType &operator=(const XSDType &other);

    void setContentModel(ContentModel contentModel);
    ContentModel contentModel() const;

    /**
     * Sets the name of the substitution element associated with this type.
     *
     * Example: <xs:element name="FieldURI" type="t:PathToUnindexedFieldType" substitutionGroup="t:Path"/>
     * will set the element name to "FieldURI" in the type "PathToUnindexedFieldType".
     *
     * @param name element name
     */
    void setSubstitutionElementName(const QName &name);
    /**
     * @return the substitution element name associated with this type, if any.
     */
    QName substitutionElementName() const;

    virtual bool isSimple() const
    {
        return true;
    }

private:
    class Private;
    Private *d;
};

}

#endif
