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

#ifndef KWSDL_SERVICE_H
#define KWSDL_SERVICE_H

#include <QDomElement>
#include <QList>

#include <wsdl/binding.h>
#include <wsdl/element.h>
#include <wsdl/port.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL
{

class KWSDL_EXPORT Service : public Element
{
public:
    typedef QList<Service> List;

    Service();
    Service(const QString &nameSpace);
    ~Service();

    void setName(const QString &name);
    QString name() const;

    void setPorts(const Port::List &ports);
    Port::List ports() const;

    void loadXML(ParserContext *context, Binding::List *bindings, const QDomElement &element);
    void saveXML(ParserContext *context, const Binding::List *bindings, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    Port::List mPorts;
};

}

#endif // KWSDL_SERVICE_H

