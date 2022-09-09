/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_WSDL_H
#define KWSDL_WSDL_H

#include <common/nsmanager.h>
#include <kode_export.h>
#include <wsdl/definitions.h>

namespace KWSDL {

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
