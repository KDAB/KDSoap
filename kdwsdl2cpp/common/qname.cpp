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

#include "qname.h"
#include <QDebug>

QName::QName()
{
}

QName::QName( const QString &name )
    : mNameSpace( "" ),
      mLocalName( "" ),
      mPrefix( "" )
{
    parse( name );
}

QName::QName( const QString &nameSpace, const QString &name )
    : mNameSpace( nameSpace ),
      mLocalName( "" ),
      mPrefix( "" )
{
    // if localName contains a ':' prefix will also be correctly extract else mLocalName will be set to localName
    parse( localName );
}

void QName::operator=( const QString &name )
{
  parse( name );
}

QString QName::localName() const
{
  return mLocalName;
}

QString QName::prefix() const
{
  return mPrefix;
}

QString QName::qname() const
{
  if ( mPrefix.isEmpty() )
    return mLocalName;
  else
    return mPrefix + QLatin1Char(':') + mLocalName;
}

void QName::setNameSpace( const QString &nameSpace )
{
  mNameSpace = nameSpace;
}

QString QName::nameSpace() const
{
  return mNameSpace;
}

bool QName::operator==( const QName &qname ) const
{
  return (qname.nameSpace() == mNameSpace && qname.localName() == mLocalName);
}

bool QName::operator!=( const QName &qname ) const
{
  return !operator==( qname );
}

bool QName::isEmpty() const
{
  return (mNameSpace.isEmpty() && mLocalName.isEmpty());
}

void QName::parse( const QString &str )
{
  int pos = str.indexOf( QLatin1Char(':') );
  if ( pos != -1 ) {
    mPrefix = str.left( pos );
    mLocalName = str.mid( pos + 1 );
  } else {
    mLocalName = str;
  }
  Q_ASSERT(!mLocalName.contains(QLatin1Char(':')));
}

QDebug operator <<(QDebug dbg, const QName &qn)
{
    if (qn.prefix().isEmpty())
        dbg << "(" << qn.nameSpace() << "," << qn.localName() << ")";
    else
        dbg << qn.qname();
    return dbg;
}
