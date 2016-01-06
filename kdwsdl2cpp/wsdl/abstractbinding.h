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

#ifndef KWSDL_ABSTRACTBINDING_H
#define KWSDL_ABSTRACTBINDING_H

#include <qdom.h>

#include <common/qname.h>
#include <wsdl/bindingoperation.h>

class ParserContext;

namespace KWSDL
{

class AbstractBinding
{
public:
    virtual ~AbstractBinding() {}

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
