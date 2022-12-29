/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_ABSTRACTBINDING_H
#define KWSDL_ABSTRACTBINDING_H

#include <qdom.h>

#include <common/qname.h>
#include <wsdl/bindingoperation.h>

class ParserContext;

namespace KWSDL {

class AbstractBinding
{
public:
    AbstractBinding() = default;
    AbstractBinding(const AbstractBinding &other) = delete;
    AbstractBinding &operator=(const AbstractBinding &other) = delete;
    virtual ~AbstractBinding()
    {
    }

    virtual void parseBinding(ParserContext *context, const QDomElement &parent) = 0;
    virtual void parseOperation(ParserContext *context, const QString &name, const QDomElement &parent) = 0;
    virtual void parseOperationInput(ParserContext *context, const QString &name, const QDomElement &parent) = 0;
    virtual void parseOperationOutput(ParserContext *context, const QString &name, const QDomElement &parent) = 0;
    virtual void parseOperationFault(ParserContext *context, const QString &name, const QDomElement &parent) = 0;
    virtual void parsePort(ParserContext *context, const QDomElement &parent) = 0;

    virtual void synthesizeBinding(ParserContext *context, QDomDocument &document, QDomElement &parent) const = 0;
    virtual void synthesizeOperation(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const = 0;
    virtual void synthesizeOperationInput(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const = 0;
    virtual void synthesizeOperationOutput(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const = 0;
    virtual void synthesizeOperationFault(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const = 0;
    virtual void synthesizePort(ParserContext *context, QDomDocument &document, QDomElement &parent) const = 0;
};

}

#endif // KWSDL_ABSTRACTBINDING_H
