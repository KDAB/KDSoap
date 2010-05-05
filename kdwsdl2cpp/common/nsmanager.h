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

#ifndef NSMANAGER_H
#define NSMANAGER_H

#include <QMap>
#include <QStringList>

#include <common/qname.h>
#include <kode_export.h>
class QDomElement;

class KXMLCOMMON_EXPORT NSManager
{
  public:
    NSManager();

    void setCurrentNamespace( const QString& uri );
    void setPrefix( const QString &prefix, const QString &uri );

    QString prefix( const QString &uri ) const;
    QString uri( const QString &prefix ) const;

    QString fullName( const QString &nameSpace, const QString &localname ) const;
    QString fullName( const QName &name ) const;

    void enterChild( const QDomElement& element );
    void exitChild( const QDomElement& element );
    QString nameSpace( const QDomElement& element ) const;
    QString localName( const QDomElement& element ) const;

    QStringList prefixes() const;
    QStringList uris() const;

    void reset();

    QString schemaPrefix() const;
    QString schemaInstancePrefix() const;
    QString soapEncPrefix() const;

    void dump() const;

  private:
    void splitName( const QString &qname, QString &prefix, QString &localname ) const;

    QMultiMap<QString, QString> mMap;
    QString mCurrentNamespace;
};

#endif

