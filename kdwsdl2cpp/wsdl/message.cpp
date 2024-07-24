/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include "message.h"

#include <QDebug>
#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>

using namespace KWSDL;

Message::Message()
{
}

Message::Message(const QString &nameSpace)
    : Element(nameSpace)
{
}

Message::~Message()
{
}

void Message::setName(const QString &name)
{
    mName = name;
}

QString Message::name() const
{
    return mName;
}

void Message::setParts(const Part::List &parts)
{
    mParts = parts;
}

Part::List Message::parts() const
{
    return mParts;
}

Part Message::partByName(const QString &name) const
{
    for (const Part &part : std::as_const(mParts)) {
        if (part.name() == name) { // # namespace comparison needed too?
            return part;
        }
    }
    qDebug() << "Part not found" << name << "in message" << mName;
    return Part();
}

void Message::loadXML(ParserContext *context, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    if (mName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Message: 'name' required"));
    }

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        const QName tagName(child.tagName());
        if (tagName.localName() == QLatin1String("part")) {
            Part part; // not part(nameSpace()), we'll use the body namespace (github issue #101)
            part.loadXML(context, child);
            mParts.append(part);
        } else if (tagName.localName() == QLatin1String("documentation")) {
            setDocumentation(child.text().trimmed());
        } else {
            context->messageHandler()->warning(QString::fromLatin1("Message: unknown tag %1").arg(child.tagName()));
        }

        child = child.nextSiblingElement();
    }
}

void Message::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("message"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    } else {
        context->messageHandler()->warning(QLatin1String("Message: 'name' required"));
    }

    for (const Part &part : std::as_const(mParts)) {
        part.saveXML(context, document, element);
    }
}
