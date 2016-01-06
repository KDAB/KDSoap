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

#ifndef KWSDL_PORTTYPE_H
#define KWSDL_PORTTYPE_H

#include <QDomElement>
#include <QList>

#include <wsdl/element.h>
#include <wsdl/operation.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL
{

class KWSDL_EXPORT PortType : public Element
{
public:
    typedef QList<PortType> List;

    PortType();
    PortType(const QString &nameSpace);
    ~PortType();

    void setName(const QString &name);
    QString name() const;

    void setOperations(const Operation::List &operations);
    Operation::List operations() const;

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    Operation::List mOperations;
};

}

#endif // KWSDL_PORTTYPE_H

