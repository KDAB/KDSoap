/*
    This file is part of KDE.

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

#ifndef KWSDL_TYPEMAP_H
#define KWSDL_TYPEMAP_H

#include <QStringList>

#include <common/qname.h>
#include <schema/types.h>

static const QString XMLSchemaURI( "http://www.w3.org/2001/XMLSchema" );
static const QString XMLSchemaInstanceURI( "http://www.w3.org/2001/XMLSchema-instance" );

class NSManager;

namespace KWSDL {

class TypeMap
{
  public:

    TypeMap();
    ~TypeMap();

    void setNSManager( NSManager *manager );

    bool isBasicType( const QName &typeName );
    bool isBuiltinType( const QName &typeName );

    QString localType( const QName &typeName );
    QStringList headers( const QName &typeName );
    QStringList forwardDeclarations( const QName &typeName );
    QStringList headerIncludes( const QName &typeName );
    QString localNameSpace( const QName &typeName );

    QString localTypeForElement( const QName &typeName );
    QStringList headersForElement( const QName &typeName );
    QStringList forwardDeclarationsForElement( const QName &typeName );
    QString localNameSpaceForElement( const QName &typeName );

    QString localTypeForAttribute( const QName &typeName );
    QStringList headersForAttribute( const QName &typeName );
    QStringList forwardDeclarationsForAttribute( const QName &typeName );
    QString localNameSpaceForAttribute( const QName &typeName );

    void addSchemaTypes( const XSD::Types &types );

    /// Returns the type to use for an 'input' parameter,
    /// for instance "const QString&" for QString.
    QString inputType( const QString& localType, bool isElement ) const;

    void dump();

  private:
    class Entry
    {
      public:
        bool basicType;
        bool buildinType;
        QString nameSpace;
        QString typeName;
        QString localType;
        QStringList headers;
        QStringList forwardDeclarations;
        QStringList headerIncludes;
    };

    QList<Entry>::ConstIterator typeEntry( const QName &typeName ) const;
    QList<Entry>::ConstIterator elementEntry( const QName &typeName ) const;

    QList<Entry> mTypeMap;
    QList<Entry> mElementMap;
    QList<Entry> mAttributeMap;

    NSManager *mNSManager;
};

}

#endif
