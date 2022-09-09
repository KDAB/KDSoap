/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include "binding.h"
#include "port.h"

#include <QDebug>

using namespace KWSDL;

Port::Port()
{
}

Port::Port(const QString &nameSpace)
    : Element(nameSpace)
{
}

Port::~Port()
{
}

void Port::setName(const QString &name)
{
    mName = name;
}

QString Port::name() const
{
    return mName;
}

void Port::setBindingName(const QName &bindingName)
{
    mBindingName = bindingName;
}

QName Port::bindingName() const
{
    return mBindingName;
}

void Port::loadXML(ParserContext *context, Binding::List *bindings, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    if (mName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Port: 'name' required"));
    }

    mBindingName = element.attribute(QLatin1String("binding"));
    if (mBindingName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Port: 'binding' required"));
    } else if (mBindingName.nameSpace().isEmpty()) { // well, always true...
        mBindingName.setNameSpace(nameSpace());
    }

    for (int i = 0; i < bindings->count(); ++i) {
        if ((*bindings)[i].name() == mBindingName.localName()) {
            AbstractBinding *binding = const_cast<AbstractBinding *>((*bindings)[i].binding());
            if (binding) {
                binding->parsePort(context, element);
            }
            // else // ignore unimplemented bindings
            //  context->messageHandler()->error( "No binding set" );
        }
    }
}

void Port::saveXML(ParserContext *context, const Binding::List *bindings, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("port"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    } else {
        context->messageHandler()->warning(QLatin1String("Port: 'name' required"));
    }

    if (!mBindingName.isEmpty()) {
        element.setAttribute(QLatin1String("binding"), mBindingName.qname());
    } else {
        context->messageHandler()->warning(QLatin1String("Port: 'binding' required"));
    }

    for (int i = 0; i < bindings->count(); ++i) {
        if ((*bindings)[i].name() == mBindingName.localName()) {
            const AbstractBinding *binding = (*bindings)[i].binding();

            if (binding) {
                binding->synthesizePort(context, document, element);
            }
            // else // ignore unimplemented bindings
            //  context->messageHandler()->error( "No binding set" );
        }
    }
}
