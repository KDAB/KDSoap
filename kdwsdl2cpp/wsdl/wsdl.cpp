/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include "wsdl.h"
#include <QDebug>

using namespace KWSDL;

WSDL::WSDL()
{
}

WSDL::~WSDL()
{
}

void WSDL::setDefinitions(const Definitions &definitions)
{
    mDefinitions = definitions;
}

Definitions WSDL::definitions() const
{
    return mDefinitions;
}

void WSDL::setNamespaceManager(const NSManager &namespaceManager)
{
    mNamespaceManager = namespaceManager;
}

const NSManager &WSDL::namespaceManager() const
{
    return mNamespaceManager;
}

Binding WSDL::findBinding(const QName &bindingName) const
{
    const Binding::List list = mDefinitions.bindings();
    for (const Binding &binding : list) {
        if (binding.name() == bindingName.localName() && binding.nameSpace() == bindingName.nameSpace()) {
            return binding;
        }
    }

    qDebug() << "ERROR: binding not found" << bindingName.localName() << bindingName.nameSpace();
    return Binding();
}

BindingOperation WSDL::findBindingOperation(const Binding &binding, const QString &operationName)
{
    const BindingOperation::List list = binding.operations();
    for (const BindingOperation &operation : list) {
        if (operation.name() == operationName) {
            return operation;
        }
    }
    qDebug("findBindingOperation: no match found for '%s'!", qPrintable(operationName));

    return BindingOperation();
}

PortType WSDL::findPortType(const QName &portTypeName) const
{
    // qDebug() << "Looking for portType" << portTypeName.nameSpace() << portTypeName.localName();
    const PortType::List list = mDefinitions.portTypes();
    for (const PortType &portType : list) {
        // qDebug() << "available portType:" << portType.nameSpace() << portType.name();
        if (portType.name() == portTypeName.localName() && portType.nameSpace() == portTypeName.nameSpace()) {
            return portType;
        }
    }
    qDebug("findPortType: no match found for '%s'!", qPrintable(portTypeName.qname()));

    return PortType();
}

Message WSDL::findMessage(const QName &messageName) const
{
    const Message::List list = mDefinitions.messages();
    for (const Message &message : list) {
        // qDebug() << message.name() << message.nameSpace();
        if (message.name() == messageName.localName() && message.nameSpace() == messageName.nameSpace()) {
            return message;
        }
    }
    qDebug() << "findMessage: no match found for" << messageName.qname() << "(localName=" << messageName.localName()
             << " nameSpace=" << messageName.nameSpace() << ")";

    return Message();
}

XSD::Element WSDL::findElement(const QName &elementName) const
{
    // qDebug() << "findElement" << elementName.nameSpace() << elementName.localName();
    const XSD::Types types = mDefinitions.type().types();
    const XSD::Element::List elements = types.elements();
    for (int i = 0; i < elements.count(); ++i) {
        // qDebug() << "    " << i << elements[ i ].nameSpace() << elements[i].name();
        if (elements[i].nameSpace() == elementName.nameSpace() && elements[i].name() == elementName.localName()) {
            return elements[i];
        }
    }

    return XSD::Element();
}

XSD::ComplexType WSDL::findComplexType(const QName &typeName) const
{
    return mDefinitions.type().types().complexType(typeName);
}

QSet<QName> WSDL::uniqueBindings(const Service &service) const
{
    QSet<QName> bindings;
    const Port::List servicePorts = service.ports();
    for (const Port &port : servicePorts) {
        const Binding binding = findBinding(port.bindingName());
        if (binding.type() == Binding::SOAPBinding) {
            bindings.insert(port.bindingName());
        } else {
            // ignore non-SOAP bindings, like HTTP GET and HTTP POST
            continue;
        }
    }
    return bindings;
}
