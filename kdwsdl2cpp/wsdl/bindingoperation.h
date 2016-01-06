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

#ifndef KWSDL_BINDINGOPERATION_H
#define KWSDL_BINDINGOPERATION_H

#include <QDomElement>
#include <QList>

#include <common/qname.h>
#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL
{

class AbstractBinding;

class KWSDL_EXPORT BindingOperation : public Element
{
public:
    typedef QList<BindingOperation> List;

    BindingOperation();
    BindingOperation(const QString &nameSpace);
    ~BindingOperation();

    void setName(const QString &name);
    QString name() const;

    void loadXML(AbstractBinding *binding, ParserContext *context,  const QDomElement &element);
    void saveXML(const AbstractBinding *binding, ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
};

}

#endif

