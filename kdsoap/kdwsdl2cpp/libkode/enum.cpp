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

#include <QtCore/QStringList>

#include "enum.h"

using namespace KODE;

class Enum::Private
{
  public:
    Private()
      : mCombinable( false )
    {
    }

    QString mName;
    QStringList mEnums;
    bool mCombinable;
};

Enum::Enum()
  : d( new Private )
{
}

Enum::Enum( const Enum &other )
  : d( new Private )
{
  *d = *other.d;
}

Enum::Enum( const QString &name, const QStringList &enums, bool combinable )
  : d( new Private )
{
  d->mName = name;
  d->mEnums = enums;
  d->mCombinable = combinable;
}

Enum::~Enum()
{
  delete d;
}

Enum& Enum::operator=( const Enum &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

QString Enum::declaration() const
{
  QString retval( "enum " + d->mName + " {" );
  uint value = 0;
  QStringList::ConstIterator it;
  for ( it = d->mEnums.constBegin(); it != d->mEnums.constEnd(); ++it, ++value ) {
    if ( d->mCombinable ) {
      if ( it == d->mEnums.constBegin() )
        retval += QString( " %1 = %2" ).arg( *it ).arg( 1 << value );
      else
        retval += QString( ", %1 = %2" ).arg( *it ).arg( 1 << value );
    } else {
      if ( it == d->mEnums.constBegin() )
        retval += ' ' + *it;
      else
        retval += ", " + *it;
    }
  }

  retval += " };";

  return retval;
}
