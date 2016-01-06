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
    Binding::List::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        if ((*it).name() == bindingName.localName() && (*it).nameSpace() == bindingName.nameSpace()) {
            return *it;
        }
    }

    qDebug() << "ERROR: binding not found" << bindingName.localName() << bindingName.nameSpace();
    return Binding();
}

BindingOperation WSDL::findBindingOperation(const Binding &binding, const QString &operationName)
{
    BindingOperation::List list = binding.operations();
    BindingOperation::List::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        if ((*it).name() == operationName) {
            return *it;
        }
    }
    qDebug("findBindingOperation: no match found for '%s'!", qPrintable(operationName));

    return BindingOperation();
}

PortType WSDL::findPortType(const QName &portTypeName) const
{
    // qDebug() << "Looking for portType" << portTypeName.nameSpace() << portTypeName.localName();
    const PortType::List list = mDefinitions.portTypes();
    PortType::List::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        // qDebug() << "available portType:" << (*it).nameSpace() << (*it).name();
        if ((*it).name() == portTypeName.localName() && (*it).nameSpace() == portTypeName.nameSpace()) {
            return *it;
        }
    }
    qDebug("findPortType: no match found for '%s'!", qPrintable(portTypeName.qname()));

    return PortType();
}

Message WSDL::findMessage(const QName &messageName) const
{
    const Message::List list = mDefinitions.messages();
    Message::List::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        //qDebug() << (*it).name() << (*it).nameSpace();
        if ((*it).name() == messageName.localName() && (*it).nameSpace() == messageName.nameSpace()) {
            return *it;
        }
    }
    qDebug() << "findMessage: no match found for" << messageName.qname()
             << "(localName=" << messageName.localName() << " nameSpace=" << messageName.nameSpace() << ")";

    return Message();
}

XSD::Element WSDL::findElement(const QName &elementName) const
{
    //qDebug() << "findElement" << elementName.nameSpace() << elementName.localName();
    const XSD::Types types = mDefinitions.type().types();
    const XSD::Element::List elements = types.elements();
    for (int i = 0; i < elements.count(); ++i) {
        //qDebug() << "    " << i << elements[ i ].nameSpace() << elements[i].name();
        if (elements[ i ].nameSpace() == elementName.nameSpace() && elements[ i ].name() == elementName.localName()) {
            return elements[ i ];
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
    Q_FOREACH (const Port &port, service.ports()) {
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

