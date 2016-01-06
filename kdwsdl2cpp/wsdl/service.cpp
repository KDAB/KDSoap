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

#include "service.h"

#include <common/messagehandler.h>
#include <common/parsercontext.h>
#include <common/nsmanager.h>

#include <QDebug>

using namespace KWSDL;

Service::Service()
{
}

Service::Service(const QString &nameSpace)
    : Element(nameSpace)
{
}

Service::~Service()
{
}

void Service::setName(const QString &name)
{
    mName = name;
}

QString Service::name() const
{
    return mName;
}

void Service::setPorts(const Port::List &ports)
{
    mPorts = ports;
}

Port::List Service::ports() const
{
    return mPorts;
}

void Service::loadXML(ParserContext *context, Binding::List *bindings, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    if (mName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Service: 'name' required"));
    }

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        const QName tagName(child.tagName());
        if (tagName.localName() == QLatin1String("port")) {
            Port port(nameSpace());
            port.loadXML(context, bindings, child);
            mPorts.append(port);
        } else if (tagName.localName() == QLatin1String("documentation")) {
            const QString text = child.text().trimmed();
            setDocumentation(text);
        } else {
            context->messageHandler()->warning(QString::fromLatin1("Service: unknown tag %1").arg(child.tagName()));
        }

        child = child.nextSiblingElement();
    }
}

void Service::saveXML(ParserContext *context, const Binding::List *bindings, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("service"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    } else {
        context->messageHandler()->warning(QLatin1String("Service: 'name' required"));
    }

    Port::List::ConstIterator it(mPorts.begin());
    const Port::List::ConstIterator endIt(mPorts.end());
    for (; it != endIt; ++it) {
        (*it).saveXML(context, bindings, document, element);
    }
}
