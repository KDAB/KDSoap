/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "style.h"

using namespace KODE;

class Style::Private
{
  public:
};

Style::Style()
  : d( 0 )
{
}

Style::Style( const Style &/*other*/ )
  : d( 0 )
{
//  *d = *other.d;
}

Style::~Style()
{
  delete d;
}

Style& Style::operator=( const Style &other )
{
  if ( this == &other )
    return *this;

  // *d = *other,d;

  return *this;
}

QString Style::className( const QString &str )
{
  Q_ASSERT(!str.isEmpty());
  QString cl = upperFirst( str );
  cl.replace(QLatin1Char('-'), QLatin1Char('_'));
  cl.replace(QLatin1Char(';'), QLatin1Char('_'));
  cl.replace(QLatin1Char(':'), QLatin1Char('_'));
  return cl;
}

QString Style::upperFirst( const QString &str )
{
  if ( str.isEmpty() )
    return str;

  return str[ 0 ].toUpper() + str.mid( 1 );
}

QString Style::lowerFirst( const QString &str )
{
  if ( str.isEmpty() )
    return str;

  return str[ 0 ].toLower() + str.mid( 1 );
}
