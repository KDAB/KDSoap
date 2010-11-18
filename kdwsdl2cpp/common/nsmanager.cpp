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

#include "nsmanager.h"

#include <QDebug>
#include <QDomElement>

// maybe port to QXmlNamespaceSupport?

NSManager::NSManager()
{
}

void NSManager::setCurrentNamespace( const QString& uri )
{
    mCurrentNamespace = uri;
}

void NSManager::setPrefix( const QString &prefix, const QString &uri )
{
  // Note that it's allowed to have two prefixes for the same namespace uri.
  //qDebug() << "NSManager::setPrefix" << uri << "->" << prefix;
  if ( !mMap.contains( uri, prefix ) ) {
    mMap.insert( uri, prefix );
  }
}

QString NSManager::prefix( const QString &uri ) const
{
  return mMap.value( uri );
}

QString NSManager::uri( const QString &prefix ) const
{
    if (prefix.isEmpty())
        return mCurrentNamespace;
    return mMap.key( prefix ); // linear search
}

void NSManager::splitName( const QString &qname, QString &prefix, QString &localname ) const
{
  int pos = qname.indexOf( ':' );
  if ( pos != -1 ) {
    prefix = qname.left( pos );
    localname = qname.mid( pos + 1 );
  } else {
    prefix = QString();
    localname = qname;
  }
}

QString NSManager::fullName( const QString &nameSpace, const QString &localname ) const
{
  if ( prefix( nameSpace ).isEmpty() )
    return localname;
  else
    return prefix( nameSpace ) + ':' + localname;
}

QString NSManager::fullName( const QName &name ) const
{
  return fullName( name.nameSpace(), name.localName() );
}

QStringList NSManager::prefixes() const
{
  return mMap.values();
}

QStringList NSManager::uris() const
{
  return mMap.keys();
}

QString NSManager::schemaPrefix() const
{
  return prefix( "http://www.w3.org/2001/XMLSchema" );
}

QString NSManager::schemaInstancePrefix() const
{
  return prefix( "http://www.w3.org/2001/XMLSchema-instance" );
}

QString NSManager::soapEncPrefix() const
{
  return prefix( "http://schemas.xmlsoap.org/soap/encoding/" );
}

void NSManager::reset()
{
  mMap.clear();
}

void NSManager::dump() const
{
  QMap<QString, QString>::ConstIterator it;
  for ( it = mMap.begin(); it != mMap.end(); ++it ) {
    qDebug( "%s\t%s", qPrintable( it.value() ), qPrintable( it.key() ) );
  }
}

QString NSManager::nameSpace(const QDomElement &element) const
{
    if (!element.namespaceURI().isEmpty()) // namespace support was enabled in QDom: easy
        return element.namespaceURI();

    QString prefix, localname;
    splitName( element.tagName(), prefix, localname );
    return uri( prefix );
}

QString NSManager::localName(const QDomElement &element) const
{
    if (!element.namespaceURI().isEmpty()) // namespace support was enabled in QDom: easy
        return element.localName();

    QString prefix, localname;
    splitName( element.tagName(), prefix, localname );
    return localname;
}

void NSManager::enterChild(const QDomElement &element)
{
    const QDomNamedNodeMap attributes = element.attributes();
    for ( int i = 0; i < attributes.count(); ++i ) {
      QDomAttr attribute = attributes.item( i ).toAttr();
      if ( attribute.name().startsWith( QLatin1String("xmlns:") ) ) {
        QString prefix = attribute.name().mid( 6 );
        setPrefix( prefix, attribute.value() );
      } else if ( attribute.name() == "xmlns" ) {
        setCurrentNamespace( attribute.value() );
      }
    }
}

void NSManager::exitChild(const QDomElement &)
{
    // TODO: pop everything that enterChild did when entering that element.
    // We need a stack somehow...
}
