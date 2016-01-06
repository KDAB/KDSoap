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
#include <QDebug>

namespace XSD
{

class SimpleType::Private
{
public:
    Private()
        : mFacetId(NONE), mAnonymous(false),
          mSubType(TypeRestriction)
    {}

    QString mDocumentation;
    QName mBaseTypeName;
    int mFacetId;
    bool mAnonymous;
    QStringList mEnums;
    SubType mSubType;

    QName mListTypeName;

    struct FacetValueType {
        FacetValueType()
            : length(0), wsp(PRESERVE), tot(0), frac(0)
        {}
        int length;
        struct LenRange {
            LenRange() : minlen(-1), maxlen(-1) {}
            int minlen, maxlen;
        } lenRange;
        WhiteSpaceType wsp;
        struct ValueRange {
            ValueRange() : maxinc(-1), mininc(-1), maxex(-1), minex(-1) {}
            int maxinc, mininc, maxex, minex;
        } valRange;
        int tot;
        int frac;
        QString pattern;
    };

    FacetValueType mFacetValue;
};

SimpleType::SimpleType()
    : XSDType(), d(new Private)
{
}

SimpleType::SimpleType(const QString &nameSpace)
    : XSDType(nameSpace), d(new Private)
{
}

SimpleType::SimpleType(const SimpleType &other)
    : XSDType(other), d(new Private)
{
    *d = *other.d;
}

SimpleType::~SimpleType()
{
    delete d;
}

SimpleType &SimpleType::operator=(const SimpleType &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;

    return *this;
}

void SimpleType::setDocumentation(const QString &documentation)
{
    d->mDocumentation = documentation;
}

QString SimpleType::documentation() const
{
    return d->mDocumentation;
}

void SimpleType::setBaseTypeName(const QName &baseTypeName)
{
    d->mBaseTypeName = baseTypeName;
}

QName SimpleType::baseTypeName() const
{
    return d->mBaseTypeName;
}

void SimpleType::setSubType(SubType subType)
{
    d->mSubType = subType;
}

SimpleType::SubType SimpleType::subType() const
{
    return d->mSubType;
}

void SimpleType::setListTypeName(const QName &name)
{
    d->mListTypeName = name;
}

QName SimpleType::listTypeName() const
{
    return d->mListTypeName;
}

void SimpleType::setAnonymous(bool anonymous)
{
    d->mAnonymous = anonymous;
}

bool SimpleType::isAnonymous() const
{
    return d->mAnonymous;
}

SimpleType::FacetType SimpleType::parseFacetId(const QString &facet) const
{
    if (d->mBaseTypeName.isEmpty()) {
        qDebug("parseFacetId: Unknown base type");
        return NONE;
    }

    if (facet == QLatin1String("length")) {
        return LENGTH;
    } else if (facet == QLatin1String("minLength")) {
        return MINLEN;
    } else if (facet == QLatin1String("maxLength")) {
        return MAXLEN;
    } else if (facet == QLatin1String("enumeration")) {
        return ENUM;
    } else if (facet == QLatin1String("whiteSpace")) {
        return WSP;
    } else if (facet == QLatin1String("pattern")) {
        return PATTERN;
    } else if (facet == QLatin1String("maxInclusive")) {
        return MAXINC;
    } else if (facet == QLatin1String("maxExclusive")) {
        return MAXEX;
    } else if (facet == QLatin1String("minInclusive")) {
        return MININC;
    } else if (facet == QLatin1String("minExclusive")) {
        return MINEX;
    } else if (facet == QLatin1String("totalDigits")) {
        return TOT;
    } else if (facet == QLatin1String("fractionDigits")) {
        return FRAC;
    } else {
        qDebug("Unknown facet: %s", qPrintable(facet));
        return NONE;
    }
}

void SimpleType::setFacetValue(FacetType ft, const QString &value)
{
    d->mFacetId |= ft;

    if (ft == ENUM) {
        d->mEnums.append(value);
    } else if (ft == PATTERN) {
        d->mFacetValue.pattern = value;
    } else if (ft == WSP) {
        if (value == QLatin1String("preserve")) {
            d->mFacetValue.wsp = PRESERVE;
        } else if (value == QLatin1String("collapse")) {
            d->mFacetValue.wsp = COLLAPSE;
        } else if (value == QLatin1String("replace")) {
            d->mFacetValue.wsp = REPLACE;
        } else {
            qDebug("Invalid facet value for whitespace");
            return;
        }
    } else {
        const int number = value.toInt();

        if (ft == MAXEX) {
            d->mFacetValue.valRange.maxex = number;
        } else if (ft == MAXINC) {
            d->mFacetValue.valRange.maxinc = number;
        } else if (ft == MININC) {
            d->mFacetValue.valRange.mininc = number;
        } else if (ft == MINEX) {
            d->mFacetValue.valRange.minex = number;
        } else if (ft == LENGTH) {
            d->mFacetValue.length = number;
        } else if (ft == MINLEN) {
            d->mFacetValue.lenRange.minlen = number;
        } else if (ft == MAXLEN) {
            d->mFacetValue.lenRange.maxlen = number;
        } else if (ft == TOT) {
            d->mFacetValue.tot = number;
        } else if (ft == FRAC) {
            d->mFacetValue.frac = number;
        }
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
    static QName XmlAnyType(QLatin1String("http://www.w3.org/2001/XMLSchema"), QLatin1String("any"));
    return d->mSubType == TypeRestriction && d->mBaseTypeName != XmlAnyType && !d->mBaseTypeName.isEmpty()
           && !(d->mFacetId & ENUM);
}

SimpleType SimpleTypeList::simpleType(const QName &qualifiedName) const
{
    const_iterator it = constBegin();
    for (; it != constEnd(); ++it)
        if ((*it).qualifiedName() == qualifiedName) {
            return *it;
        }
    //qDebug() << "Simple type" << qualifiedName << "not found";
    return SimpleType();
}

SimpleTypeList::iterator SimpleTypeList::findSimpleType(const QName &qualifiedName)
{
    for (iterator it = begin(); it != end(); ++it)
        if ((*it).qualifiedName() == qualifiedName) {
            return it;
        }
    return end();
}

}
