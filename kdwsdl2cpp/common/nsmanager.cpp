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

NSManager::NSManager()
{
}

void NSManager::setPrefix( const QString &prefix, const QString &uri )
{
  mMap.insert( uri, prefix );
}

QString NSManager::prefix( const QString &uri ) const
{
  return mMap.value( uri );
}

QString NSManager::uri( const QString &prefix ) const
{
  QMap<QString, QString>::ConstIterator it;
  for ( it = mMap.begin(); it != mMap.end(); ++it ) {
    if ( it.value() == prefix )
      return it.key();
  }

  return QString();
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
