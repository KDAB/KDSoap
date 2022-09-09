/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>

#include "param.h"
#include <QDebug>

using namespace KWSDL;

Param::Param()
{
}

Param::Param(const QString &nameSpace)
    : Element(nameSpace)
{
}

Param::~Param()
{
}

void Param::setName(const QString &name)
{
    mName = name;
}

QString Param::name() const
{
    return mName;
}

void Param::setMessage(const QName &message)
{
    mMessage = message;
}

QName Param::message() const
{
    return mMessage;
}

void Param::loadXML(ParserContext *context, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    mMessage = element.attribute(QLatin1String("message"));
    if (mMessage.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Param: 'message' required"));
    } else {
        if (mMessage.prefix().isEmpty()) {
            mMessage.setNameSpace(nameSpace());
        } else {
            mMessage.setNameSpace(context->namespaceManager()->uri(mMessage.prefix()));
        }
    }
}

void Param::saveXML(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(name);
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    }

    if (!mMessage.isEmpty()) {
        element.setAttribute(QLatin1String("message"), mMessage.qname());
    } else {
        context->messageHandler()->warning(QLatin1String("Param: 'message' required"));
    }
}
