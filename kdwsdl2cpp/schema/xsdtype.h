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

namespace XSD {

class SCHEMA_EXPORT XSDType : public XmlElement
{
  public:
    typedef QList<const XSDType*> List;

    enum ContentModel
    {
      SIMPLE = 0,
      COMPLEX,
      MIXED
    };

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

    XSDType();
    XSDType( const QString& );
    XSDType( const XSDType &other );
    virtual ~XSDType();

    XSDType &operator=( const XSDType &other );

    void setContentModel( ContentModel contentModel);
    ContentModel contentModel() const;

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
