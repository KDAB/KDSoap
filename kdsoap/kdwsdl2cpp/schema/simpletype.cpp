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

#include "simpletype.h"

namespace XSD {

class SimpleType::Private
{
public:
    Private()
      : mFacetId( NONE ), mAnonymous( false ),
        mSubType( TypeRestriction )
    {}

    QString mDocumentation;
    QName mBaseTypeName;
    int mFacetId;
    bool mAnonymous;
    QStringList mEnums;
    SubType mSubType;

    QName mListTypeName;

    typedef struct
    {
      int length;
      struct
      {
        int minlen, maxlen;
      } lenRange;
      WhiteSpaceType wsp;
      struct
      {
        int maxinc, mininc, maxex, minex;
      } valRange;
      int tot;
      int frac;
      QString pattern;
    } FacetValueType;

    FacetValueType mFacetValue;
};

SimpleType::SimpleType()
  : XSDType(), d(new Private)
{
}

SimpleType::SimpleType( const QString &nameSpace )
  : XSDType( nameSpace ), d(new Private)
{
}

SimpleType::SimpleType( const SimpleType &other )
  : XSDType( other ), d(new Private)
{
  *d = *other.d;
}

SimpleType::~SimpleType()
{
  delete d;
}

SimpleType &SimpleType::operator=( const SimpleType &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void SimpleType::setDocumentation( const QString &documentation )
{
  d->mDocumentation = documentation;
}

QString SimpleType::documentation() const
{
  return d->mDocumentation;
}

void SimpleType::setBaseTypeName( const QName &baseTypeName )
{
  d->mBaseTypeName = baseTypeName;
}

QName SimpleType::baseTypeName() const
{
  return d->mBaseTypeName;
}

void SimpleType::setSubType( SubType subType )
{
  d->mSubType = subType;
}

SimpleType::SubType SimpleType::subType() const
{
  return d->mSubType;
}

void SimpleType::setListTypeName( const QName &name )
{
  d->mListTypeName = name;
}

QName SimpleType::listTypeName() const
{
  return d->mListTypeName;
}

void SimpleType::setAnonymous( bool anonymous )
{
  d->mAnonymous = anonymous;
}

bool SimpleType::isAnonymous() const
{
  return d->mAnonymous;
}

bool SimpleType::isValidFacet( const QString &facet )
{
  if ( d->mBaseTypeName.isEmpty() ) {
    qDebug( "isValidFacet:Unknown base type" );
    return false;
  }

  if ( facet == "length" )
    d->mFacetId |= LENGTH;
  else if ( facet == "minLength" )
    d->mFacetId |= MINLEN;
  else if ( facet == "maxLength" )
    d->mFacetId |= MAXLEN;
  else if ( facet == "enumeration" )
    d->mFacetId |= ENUM;
  else if ( facet == "whiteSpace" )
    d->mFacetId |= WSP;
  else if ( facet == "pattern" )
    d->mFacetId |= PATTERN;
  else if ( facet == "maxInclusive" )
    d->mFacetId |= MAXINC;
  else if ( facet == "maxExclusive" )
    d->mFacetId |= MAXEX;
  else if ( facet == "minInclusive" )
    d->mFacetId |= MININC;
  else if ( facet == "minExclusive" )
    d->mFacetId |= MINEX;
  else if ( facet == "totalDigits" )
    d->mFacetId |= TOT;
  else if ( facet == "fractionDigits" )
    d->mFacetId |= FRAC;
  else {
    d->mFacetId = NONE;
    return false;
  }

  return true;
}

void SimpleType::setFacetValue( const QString &value )
{
  int number = -1;

  if ( d->mFacetId & ENUM ) {
    d->mEnums.append( value );
  } else if ( d->mFacetId & PATTERN ) {
    d->mFacetValue.pattern = value;
  } else if ( d->mFacetId & WSP ) {
    if ( value == "preserve" )
      d->mFacetValue.wsp = PRESERVE;
    else if ( value == "collapse" )
      d->mFacetValue.wsp = COLLAPSE;
    else if ( value == "replace" )
      d->mFacetValue.wsp = REPLACE;
    else {
      qDebug( "Invalid facet value for whitespace" );
      return;
    }
  } else {
    number = value.toInt();
  }

  if ( d->mFacetId & MAXEX ) {
    d->mFacetValue.valRange.maxex = number;
  } else if ( d->mFacetId & MAXINC ) {
    d->mFacetValue.valRange.maxinc = number;
  } else if ( d->mFacetId & MININC ) {
    d->mFacetValue.valRange.mininc = number;
  } else if ( d->mFacetId & MINEX ) {
    d->mFacetValue.valRange.minex = number;
  } else if ( d->mFacetId & MAXEX ) {
    d->mFacetValue.valRange.maxex = number;
  } else if ( d->mFacetId & LENGTH ) {
    d->mFacetValue.length = number;
  } else if ( d->mFacetId & MINLEN ) {
    d->mFacetValue.lenRange.minlen = number;
  } else if ( d->mFacetId & MAXLEN ) {
    d->mFacetValue.lenRange.maxlen = number;
  } else if ( d->mFacetId & TOT ) {
    d->mFacetValue.tot = number;
  } else if ( d->mFacetId & FRAC ) {
    d->mFacetValue.frac = number;
  }
}

int SimpleType::facetType() const
{
  return d->mFacetId;
}

int SimpleType::facetLength() const
{
  return d->mFacetValue.length;
}

int SimpleType::facetMinimumLength() const
{
  return d->mFacetValue.lenRange.minlen;
}

int SimpleType::facetMaximumLength() const
{
  return d->mFacetValue.lenRange.maxlen;
}

QStringList SimpleType::facetEnums() const
{
  return d->mEnums;
}

SimpleType::WhiteSpaceType SimpleType::facetWhiteSpace() const
{
  return d->mFacetValue.wsp;
}

int SimpleType::facetMinimumInclusive() const
{
  return d->mFacetValue.valRange.mininc;
}

int SimpleType::facetMaximumInclusive() const
{
  return d->mFacetValue.valRange.maxinc;
}

int SimpleType::facetMinimumExclusive() const
{
  return d->mFacetValue.valRange.minex;
}

int SimpleType::facetMaximumExclusive() const
{
  return d->mFacetValue.valRange.maxex;
}

int SimpleType::facetTotalDigits() const
{
  return d->mFacetValue.tot;
}

int SimpleType::facetFractionDigits() const
{
  return d->mFacetValue.frac;
}

QString SimpleType::facetPattern() const
{
  return d->mFacetValue.pattern;
}

bool SimpleType::isRestriction() const
{
    static QName XmlAnyType( "http://www.w3.org/2001/XMLSchema", "any" );
    return d->mSubType == TypeRestriction && d->mBaseTypeName != XmlAnyType && !d->mBaseTypeName.isEmpty()
            && !(d->mFacetId & ENUM);
}

SimpleTypeList::const_iterator SimpleTypeList::findSimpleType(const QName &qualifiedName) const
{
  const_iterator it = constBegin();
  for ( ; it != constEnd(); ++it )
    if ((*it).qualifiedName() == qualifiedName)
      break;
  return it;
}

}
