/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include "porttype.h"

#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>

using namespace KWSDL;

PortType::PortType()
{
}

PortType::PortType(const QString &nameSpace)
    : Element(nameSpace)
{
}

PortType::~PortType()
{
}

void PortType::setName(const QString &name)
{
    mName = name;
}

QString PortType::name() const
{
    return mName;
}

void PortType::setOperations(const Operation::List &operations)
{
    mOperations = operations;
}

Operation::List PortType::operations() const
{
    return mOperations;
}

void PortType::loadXML(ParserContext *context, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    if (mName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("PortType: 'name' required"));
    }

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        const QName tagName(child.tagName());
        if (tagName.localName() == QLatin1String("operation")) {
            Operation operation(nameSpace());
            operation.loadXML(context, child);
            mOperations.append(operation);
        } else if (tagName.localName() == QLatin1String("documentation")) {
            setDocumentation(child.text().trimmed());
        } else {
            context->messageHandler()->warning(QString::fromLatin1("PortType: unknown tag %1").arg(child.tagName()));
        }

        child = child.nextSiblingElement();
    }
}

void PortType::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("portType"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    } else {
        context->messageHandler()->warning(QLatin1String("PortType: 'name' required"));
    }

    for (const Operation &operation : qAsConst(mOperations)) {
        operation.saveXML(context, document, element);
    }
}
