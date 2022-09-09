/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
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
