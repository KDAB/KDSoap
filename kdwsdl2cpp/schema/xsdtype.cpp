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

#include "xsdtype.h"

namespace XSD
{

class XSDType::Private
{
public:
    Private()
        : mContentModel(SIMPLE),
          mSubstitutionElementName()
    {}

    ContentModel mContentModel;
    QName mSubstitutionElementName;
};

XSDType::XSDType()
    : XmlElement(), d(new Private)
{
}

XSDType::XSDType(const QString &nameSpace)
    : XmlElement(nameSpace), d(new Private)
{
}

XSDType::XSDType(const XSDType &other)
    : XmlElement(other), d(new Private)
{
    *d = *other.d;
}

XSDType::~XSDType()
{
    delete d;
}

XSDType &XSDType::operator=(const XSDType &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;

    return *this;
}

void XSDType::setContentModel(ContentModel contentModel)
{
    d->mContentModel = contentModel;
}

XSDType::ContentModel XSDType::contentModel() const
{
    return d->mContentModel;
}

void XSDType::setSubstitutionElementName(const QName &name)
{
    d->mSubstitutionElementName = name;
}

QName XSDType::substitutionElementName() const
{
    return d->mSubstitutionElementName;
}

} // namespace XSD
