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

#include "membervariable.h"
#include "style.h"

using namespace KODE;

class MemberVariable::Private
{
  public:
};

MemberVariable::MemberVariable()
  : Variable(), d( nullptr )
{
}

MemberVariable::MemberVariable( const MemberVariable &other )
  : Variable( other ), d( nullptr )
{
  // *d = *other.d;
}

MemberVariable::MemberVariable( const QString &name, const QString &type,
                                bool isStatic )
  : Variable( name, type, isStatic ), d( nullptr )
{
    setName( memberVariableName( name ) );
}

MemberVariable::~MemberVariable()
{
  delete d;
}

MemberVariable& MemberVariable::operator=( const MemberVariable &other )
{
  if ( this == &other )
    return *this;

  Variable::operator=( other );
  // *d = *other.d;

  return *this;
}

QString MemberVariable::memberVariableName( const QString &name )
{
    QString n;

    if ( name.isEmpty() ) {
      n = QLatin1String("mUndefined");
    } else if ( name.length() >= 2  && name[ 0 ] == QLatin1Char( 'm' ) &&
                ( name[ 1 ].toUpper() == name[ 1 ] ) ) {
      n = name;
    } else if ( name == QLatin1String("q") || name == QLatin1String("d") ) {
      n = name;
    } else {
      n = QLatin1String("m");
      n += name[ 0 ].toUpper();
      n += name.mid( 1 );
    }

    return Style::makeIdentifier(n);
}
