/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include <wsdl/abstractbinding.h>

#include "bindingoperation.h"

#include <common/messagehandler.h>
#include <common/parsercontext.h>
#include <common/nsmanager.h>

using namespace KWSDL;

BindingOperation::BindingOperation()
{
}

BindingOperation::BindingOperation(const QString &nameSpace)
    : Element(nameSpace)
{
}

BindingOperation::~BindingOperation()
{
}

void BindingOperation::setName(const QString &name)
{
    mName = name;
}

QString BindingOperation::name() const
{
    return mName;
}

void BindingOperation::loadXML(AbstractBinding *binding, ParserContext *context, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    if (mName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("BindingOperation: 'name' required"));
    }

    binding->parseOperation(context, mName, element);

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        const QName tagName(child.tagName());
        if (tagName.localName() == QLatin1String("input")) {
            binding->parseOperationInput(context, mName, child);
        } else if (tagName.localName() == QLatin1String("output")) {
            binding->parseOperationOutput(context, mName, child);
        } else if (tagName.localName() == QLatin1String("fault")) {
            binding->parseOperationFault(context, mName, child);
        } // no fallback else here, some other tags (like soap:operation) are already parsed by parseOperation().

        child = child.nextSiblingElement();
    }
}

void BindingOperation::saveXML(const AbstractBinding *binding, ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("operation"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    } else {
        context->messageHandler()->warning(QLatin1String("BindingOperation: 'name' required"));
    }

    binding->synthesizeOperation(context, mName, document, element);

    QDomElement inputElement = document.createElement(QLatin1String("input"));
    element.appendChild(inputElement);
    binding->synthesizeOperationInput(context, mName, document, inputElement);

    QDomElement outputElement = document.createElement(QLatin1String("output"));
    element.appendChild(outputElement);
    binding->synthesizeOperationOutput(context, mName, document, outputElement);

    QDomElement faultElement = document.createElement(QLatin1String("fault"));
    element.appendChild(faultElement);
    binding->synthesizeOperationFault(context, mName, document, faultElement);
}
