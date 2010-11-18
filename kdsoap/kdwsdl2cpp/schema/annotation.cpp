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
    This file is part of KDE Schema Parser

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

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

#include "annotation.h"

#include <common/qname.h>

namespace XSD {

class Annotation::Private
{
public:
   QDomElement mDomElement;
};

Annotation::Annotation()
  : d(new Private)
{
}

Annotation::Annotation( const QDomElement &element )
  : d(new Private)
{
  d->mDomElement = element;
}

Annotation::Annotation( const Annotation &other )
  : d(new Private)
{
  *d = *other.d;
}

Annotation::~Annotation()
{
  delete d;
}

Annotation &Annotation::operator=( const Annotation &other )
{
  if( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}
void Annotation::setDomElement( const QDomElement &element )
{
  d->mDomElement = element;
}

QDomElement Annotation::domElement() const
{
  return d->mDomElement;
}

bool Annotation::isDocumentation() const
{
  return QName( d->mDomElement.tagName() ).localName() == "documentation";
}

bool Annotation::isAppinfo() const
{
  return QName( d->mDomElement.tagName() ).localName() == "appinfo";
}

QString Annotation::documentation() const
{
  QString result;

  if ( isDocumentation() ) {
    result = d->mDomElement.text().trimmed();
  }

  return result;
}


QString Annotation::List::documentation() const
{
  QString result;

  foreach( Annotation a, *this ) {
    result.append( a.documentation() );
  }

  return result;
}

}
