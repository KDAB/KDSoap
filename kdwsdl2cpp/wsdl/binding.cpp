/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include <QDebug>
#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>

#include "binding.h"

using namespace KWSDL;

static const char soapStandardNamespace[] = "http://schemas.xmlsoap.org/wsdl/soap/";
static const char soap12StandardNamespace[] = "http://schemas.xmlsoap.org/wsdl/soap12/";
static const char httpStandardNamespace[] = "http://schemas.xmlsoap.org/wsdl/http/";

Binding::Binding()
    : mType(UnknownBinding)
    , mVersion(SOAP_1_1)
{
}

Binding::Binding(const QString &nameSpace)
    : Element(nameSpace)
    , mType(UnknownBinding)
    , mVersion(SOAP_1_1)
{
}

Binding::~Binding()
{
}

void Binding::loadXML(ParserContext *context, const QDomElement &element)
{
    mName = element.attribute(QLatin1String("name"));
    if (mName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Binding: 'name' required"));
    }

    mPortTypeName = element.attribute(QLatin1String("type"));
    if (mPortTypeName.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Binding: 'type' required"));
    } else {
        mPortTypeName.setNameSpace(context->namespaceManager()->uri(mPortTypeName.prefix()));
        if (mPortTypeName.nameSpace().isEmpty()) {
            mPortTypeName.setNameSpace(nameSpace());
        }
    }

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        QName tagName(child.tagName());

        tagName.setNameSpace(context->namespaceManager()->uri(tagName.prefix()));

        if (tagName.localName() == QLatin1String("operation")) {
            BindingOperation operation(nameSpace());
            operation.loadXML(&mSoapBinding, context, child);
            mOperations.append(operation);
        } else if (tagName.localName() == QLatin1String("binding")) {
            const bool soap11 = (tagName.nameSpace() == soapStandardNamespace);
            const bool soap12 = (tagName.nameSpace() == soap12StandardNamespace);
            if (soap11 || soap12) {
                if (soap12) {
                    mVersion = SOAP_1_2;
                }
                mType = SOAPBinding;
                mSoapBinding.parseBinding(context, child);
            } else if (tagName.nameSpace() == httpStandardNamespace) {
                mType = HTTPBinding;
                qWarning() << "Not implemented: HTTPBinding";
            } else {
                qWarning() << "Not implemented: MIMEBinding";
            }
        }

        child = child.nextSiblingElement();
    }
}

void Binding::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("binding"));
    parent.appendChild(element);

    if (!mName.isEmpty()) {
        element.setAttribute(QLatin1String("name"), mName);
    } else {
        context->messageHandler()->warning(QLatin1String("Binding: 'name' required"));
    }

    if (!mPortTypeName.isEmpty()) {
        element.setAttribute(QLatin1String("type"), mPortTypeName.localName());
    } else {
        context->messageHandler()->warning(QLatin1String("Binding: 'type' required"));
    }

    mSoapBinding.synthesizeBinding(context, document, element);

    for (const BindingOperation &operation : qAsConst(mOperations)) {
        operation.saveXML(&mSoapBinding, context, document, element);
    }
}

void Binding::setName(const QString &name)
{
    mName = name;
}

QString Binding::name() const
{
    return mName;
}

void Binding::setPortTypeName(const QName &portTypeName)
{
    mPortTypeName = portTypeName;
}

QName Binding::portTypeName() const
{
    return mPortTypeName;
}

void Binding::setOperations(const BindingOperation::List &operations)
{
    mOperations = operations;
}

BindingOperation::List Binding::operations() const
{
    return mOperations;
}

void Binding::setType(Type type)
{
    mType = type;
}

Binding::Type Binding::type() const
{
    return mType;
}

void Binding::setVersion(Binding::Version v)
{
    mVersion = v;
}

Binding::Version Binding::version() const
{
    return mVersion;
}

#if 0
void Binding::setSoapBinding(const SoapBinding &soapBinding)
{
    mSoapBinding = soapBinding;
}
#endif

SoapBinding Binding::soapBinding() const
{
    return mSoapBinding;
}

const AbstractBinding *Binding::binding() const
{
    if (mType == SOAPBinding) {
        return &mSoapBinding;
    } else { // Not Implemented: HTTPBinding and MIMEBinding
        return 0;
    }
}
