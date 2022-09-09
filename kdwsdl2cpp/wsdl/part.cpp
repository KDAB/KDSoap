/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include <common/nsmanager.h>
#include <common/parsercontext.h>

#include "part.h"

using namespace KWSDL;

Part::Part()
{
}

Part::Part(const QString &nameSpace)
    : Element(nameSpace)
{
}

Part::~Part()
{
}

void Part::setName(const QString &name)
{
    mName = name;
}

QString Part::name() const
{
    return mName;
}

void Part::setType(const QName &type)
{
    mType = type;
}

QName Part::type() const
{
    return mType;
}

void Part::setElement(const QName &element)
{
    mElement = element;
}

QName Part::element() const
{
    return mElement;
}

void Part::loadXML(ParserContext *context, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    mType = element.attribute(QLatin1String("type"));
    if (!mType.prefix().isEmpty()) {
        mType.setNameSpace(context->namespaceManager()->uri(mType.prefix()));
    }

    mElement = element.attribute(QLatin1String("element"));
    mElement.setNameSpace(context->namespaceManager()->uri(mElement.prefix()));
}

void Part::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    Q_UNUSED(context);

    QDomElement element = document.createElement(QLatin1String("part"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    }

    if (!mType.isEmpty()) {
        element.setAttribute(QLatin1String("type"), mType.qname());
    }

    if (!mElement.isEmpty()) {
        element.setAttribute(QLatin1String("element"), mElement.qname());
    }
}
