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
QT_BEGIN_NAMESPACE
class QDomElement;
QT_END_NAMESPACE
class ParserContext;

class KXMLCOMMON_EXPORT NSManager
{
  public:
    NSManager();
    // Called when entering a new XML element. We copy the current namespaces
    // from the context and add the ones defined by the new XML element.
    // Upon destruction, we restore the context.
    NSManager( ParserContext* context, const QDomElement& child );
    ~NSManager();

    void enterChild( const QDomElement& element );

    void setCurrentNamespace( const QString& uri );
    void setPrefix( const QString &prefix, const QString &uri );

    QString prefix( const QString &uri ) const;
    QString uri( const QString &prefix ) const;

    QString fullName( const QString &nameSpace, const QString &localname ) const;
    QString fullName( const QName &name ) const;

    QString nameSpace( const QDomElement& element ) const;
    QString localName( const QDomElement& element ) const;

    QStringList prefixes() const;
    QStringList uris() const;

    void reset();

    void dump() const;

    // Repository of namespaces
    static QStringList soapEncNamespaces();
    static QStringList soapNamespaces();

  private:
    void splitName( const QString &qname, QString &prefix, QString &localname ) const;

    typedef QMultiMap<QString, QString> NSMap; // prefix -> URI
    NSMap mMap;
    QString mCurrentNamespace;

    ParserContext* mContext;
    NSManager* mParentManager;
};

#endif
