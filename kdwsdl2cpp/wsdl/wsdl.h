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

#ifndef KWSDL_WSDL_H
#define KWSDL_WSDL_H

#include <common/nsmanager.h>
#include <wsdl/definitions.h>
#include <kode_export.h>

namespace KWSDL
{

class KWSDL_EXPORT WSDL
{
public:
    WSDL();
    ~WSDL();

    void setDefinitions(const Definitions &definitions);
    Definitions definitions() const;

    void setNamespaceManager(const NSManager &namespaceManager);
    const NSManager &namespaceManager() const;

    QSet<QName> uniqueBindings(const Service &service) const;

    Binding findBinding(const QName &bindingName) const;
    BindingOperation findBindingOperation(const Binding &binding, const QString &operationName);
    PortType findPortType(const QName &portTypeName) const;
    Message findMessage(const QName &messageName) const;
    XSD::Element findElement(const QName &elementName) const;
    XSD::ComplexType findComplexType(const QName &typeName) const;

private:
    Definitions mDefinitions;
    NSManager mNamespaceManager;
};

}

#endif
