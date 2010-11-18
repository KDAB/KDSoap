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

#include <QtCore/QString>

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
  return upperFirst( str );
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
