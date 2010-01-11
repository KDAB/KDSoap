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

#ifndef SCHEMA_ELEMENT_H
#define SCHEMA_ELEMENT_H

#include <QList>
#include <QString>

#include "xmlelement.h"
#include "compositor.h"
#include <kode_export.h>

namespace XSD {

class SCHEMA_EXPORT Element : public XmlElement
{
  public:
    typedef QList<Element> List;

    Element();
    Element( const QString &nameSpace );
    Element( const Element &other );
    ~Element();

    Element &operator=( const Element &other );

    void setType( const QName &type );
    QName type() const;

    void setDocumentation( const QString &documentation );
    QString documentation() const;

    void setGroupId( int group );
    int groupId() const;

    void setMinOccurs( int minOccurs );
    int minOccurs() const;

    void setMaxOccurs( int maxOccurs );
    int maxOccurs() const;

    void setDefaultValue( const QString &defaultValue );
    QString defaultValue() const;

    void setFixedValue( const QString &fixedValue );
    QString fixedValue() const;

    void setIsQualified( bool isQualified );
    bool isQualified() const;

    void setOccurrence( int occurrence );
    int occurrence() const;

    void setReference( const QName &reference );
    QName reference() const;

    bool isResolved() const;

    void setCompositor( const Compositor & );
    Compositor compositor() const;

  private:
    class Private;
    Private *d;
};

}

#endif
