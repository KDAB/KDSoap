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

#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include "fault.h"

using namespace KWSDL;

Fault::Fault()
{
}

Fault::Fault(const QString &nameSpace)
    : Element(nameSpace)
{
}

Fault::~Fault()
{
}

void Fault::setName(const QString &name)
{
    mName = name;
}

QString Fault::name() const
{
    return mName;
}

void Fault::setMessage(const QName &message)
{
    mMessage = message;
}

QName Fault::message() const
{
    return mMessage;
}

void Fault::loadXML(ParserContext *context, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    if (mName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Fault: 'name' required"));
    }

    mMessage = element.attribute(QLatin1String("message"));
    if (mMessage.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Fault: 'message' required"));
    }
}

void Fault::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("fault"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    } else {
        context->messageHandler()->warning(QLatin1String("Fault: 'name' required"));
    }

    if (!mMessage.isEmpty()) {
        element.setAttribute(QLatin1String("message"), mMessage.qname());
    } else {
        context->messageHandler()->warning(QLatin1String("Fault: 'message' required"));
    }
}
