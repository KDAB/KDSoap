/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include "element.h"

using namespace KWSDL;

Element::Element()
{
}

Element::Element(const QString &nameSpace)
    : mNameSpace(nameSpace)
{
}

Element::~Element()
{
}

void Element::setNameSpace(const QString &nameSpace)
{
    mNameSpace = nameSpace;
}

QString Element::nameSpace() const
{
    return mNameSpace;
}

void Element::setDocumentation(const QString &documentation)
{
    mDocumentation = documentation;
}

QString Element::documentation() const
{
    return mDocumentation;
}
