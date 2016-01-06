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

#include "definitions.h"

#include <QDir>
#include <QFile>
#include <QUrl>

#include <QXmlSimpleReader>
#include <common/nsmanager.h>
#include <common/fileprovider.h>
#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include <wsdl/port.h>

#include <QDebug>

using namespace KWSDL;

Definitions::Definitions()
{
}

Definitions::~Definitions()
{
}

void Definitions::setName(const QString &name)
{
    mName = name;
}

QString Definitions::name() const
{
    return mName;
}

void Definitions::setTargetNamespace(const QString &targetNamespace)
{
    mTargetNamespace = targetNamespace;

    mType.setNameSpace(mTargetNamespace);
}

QString Definitions::targetNamespace() const
{
    return mTargetNamespace;
}

#if 0
void Definitions::setBindings(const Binding::List &bindings)
{
    mBindings = bindings;
}
#endif

Binding::List Definitions::bindings() const
{
    return mBindings;
}

#if 0
void Definitions::setImports(const Import::List &imports)
{
    mImports = imports;
}

Import::List Definitions::imports() const
{
    return mImports;
}
#endif

void Definitions::setMessages(const Message::List &messages)
{
    mMessages = messages;
}

Message::List Definitions::messages() const
{
    return mMessages;
}

void Definitions::setPortTypes(const PortType::List &portTypes)
{
    mPortTypes = portTypes;
}

PortType::List Definitions::portTypes() const
{
    return mPortTypes;
}

#if 0
void Definitions::setService(const Service &service)
{
    mService = service;
}
#endif

Service::List Definitions::services() const
{
    return mServices;
}

void Definitions::setType(const Type &type)
{
    mType = type;
}

Type Definitions::type() const
{
    return mType;
}

bool Definitions::loadXML(ParserContext *context, const QDomElement &element)
{
    setTargetNamespace(element.attribute(QLatin1String("targetNamespace")));
    mName = element.attribute(QLatin1String("name"));

    context->namespaceManager()->enterChild(element);

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        const QName tagName(child.tagName());
        if (tagName.localName() == QLatin1String("import")) {
            QString oldTn = targetNamespace();
            QString oldName = mName;
            importDefinition(context, child.attribute(QLatin1String("location")));
            setTargetNamespace(oldTn);
            mName = oldName;
        } else if (tagName.localName() == QLatin1String("types")) {
            if (!mType.loadXML(context, child)) {
                return false;
            }
        } else if (tagName.localName() == QLatin1String("message")) {
            Message message(mTargetNamespace);
            message.loadXML(context, child);
            //qDebug() << "Definitions: found message" << message.name() << message.nameSpace();
            mMessages.append(message);
        } else if (tagName.localName() == QLatin1String("portType")) {
            PortType portType(mTargetNamespace);
            portType.loadXML(context, child);
            mPortTypes.append(portType);
        } else if (tagName.localName() == QLatin1String("binding")) {
            Binding binding(mTargetNamespace);
            binding.loadXML(context, child);
            mBindings.append(binding);
        } else if (tagName.localName() == QLatin1String("service")) {
            const QString name = child.attribute(QLatin1String("name"));
            //qDebug() << "Service:" << name << "looking for" << mWantedService;
            // is this the service we want?
            if (mWantedService.isEmpty() || mWantedService == name) {
                Service service(mTargetNamespace);
                service.loadXML(context, &mBindings, child);
                mServices.append(service);
            }
        } else if (tagName.localName() == QLatin1String("documentation")) {
            // ignore documentation for now
        } else {
            context->messageHandler()->warning(QString::fromLatin1("Definitions: unknown tag %1").arg(child.tagName()));
        }
        child = child.nextSiblingElement();
    }
    return true;
}

void Definitions::fixUpDefinitions(/*ParserContext *context, const QDomElement &element */)
{
    if (mServices.isEmpty()) {
        Q_ASSERT(!mBindings.isEmpty());
        qDebug() << "No service tag found in the wsdl file, generating one service per binding";
        Q_FOREACH (const Binding &bind, mBindings) {
            Service service(mTargetNamespace);
            service.setName(bind.name() + "Service");

            Port port(mTargetNamespace);
            port.setName(bind.name() + "Port");
            QName bindingName(bind.portTypeName().prefix() + ":" + bind.name());
            bindingName.setNameSpace(bind.nameSpace());
            port.setBindingName(bindingName);

            Port::List portList;
            portList.append(port);
            service.setPorts(portList);
            mServices.append(service);
        }
    }
}

static QUrl urlForLocation(ParserContext *context, const QString &location)
{
    QUrl url(location);
    if ((url.scheme().isEmpty() || url.scheme() == QLatin1String("file"))) {
        QDir dir(location);
        if (dir.isRelative()) {
            url = context->documentBaseUrl();
            url.setPath(url.path() + QLatin1Char('/') + location);
        }
    }
    return url;
}

void Definitions::importDefinition(ParserContext *context, const QString &location)
{
    if (location.isEmpty()) {
        context->messageHandler()->warning(QString::fromLatin1("Definitions import: location tag required: %1").arg(location));
        return;
    }
    FileProvider provider;
    QString fileName;
    const QUrl locationUrl = urlForLocation(context, location);
    qDebug("Importing wsdl definition at %s", locationUrl.toEncoded().constData());

    if (provider.get(locationUrl, fileName)) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug("Unable to open file %s", qPrintable(file.fileName()));
            return;
        }

        QXmlInputSource source(&file);
        QXmlSimpleReader reader;
        reader.setFeature(QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true);

        QDomDocument doc(QLatin1String("kwsdl"));
        QString errorMsg;
        int errorLine, errorColumn;
        bool ok = doc.setContent(&source, &reader, &errorMsg, &errorLine, &errorColumn);
        if (!ok) {
            qDebug("Error[%d:%d] %s", errorLine, errorColumn, qPrintable(errorMsg));
            return;
        }

        // prepare the new context to avoid infinite recursion
        QDomElement rootNode = doc.documentElement();
        NSManager namespaceManager(context, rootNode);

        const QName tagName(rootNode.tagName());
        if (tagName.localName() == QLatin1String("definitions")) {
            // recursivity
            context->namespaceManager()->enterChild(rootNode);

            const QUrl oldBaseUrl = context->documentBaseUrl();
            context->setDocumentBaseUrlFromFileUrl(locationUrl);

            loadXML(context, rootNode);

            context->setDocumentBaseUrl(oldBaseUrl);

        } else {
            qDebug("No definition tag found in imported wsdl file %s", locationUrl.toEncoded().constData());
        }

        file.close();

        provider.cleanUp();
    }
}

#if 0
void Definitions::saveXML(ParserContext *context, QDomDocument &document) const
{
    QDomElement element = document.createElement("definitions");
    document.appendChild(element);

    if (!mTargetNamespace.isEmpty()) {
        element.setAttribute("targetNamespace", mTargetNamespace);
    }
    if (!mName.isEmpty()) {
        element.setAttribute("name", mName);
    }

    {
        Import::List::ConstIterator it(mImports.begin());
        const Import::List::ConstIterator endIt(mImports.end());
        for (; it != endIt; ++it) {
            (*it).saveXML(context, document, element);
        }
    }

    mType.saveXML(context, document, element);

    {
        Message::List::ConstIterator it(mMessages.begin());
        const Message::List::ConstIterator endIt(mMessages.end());
        for (; it != endIt; ++it) {
            (*it).saveXML(context, document, element);
        }
    }

    {
        PortType::List::ConstIterator it(mPortTypes.begin());
        const PortType::List::ConstIterator endIt(mPortTypes.end());
        for (; it != endIt; ++it) {
            (*it).saveXML(context, document, element);
        }
    }

    {
        Binding::List::ConstIterator it(mBindings.begin());
        const Binding::List::ConstIterator endIt(mBindings.end());
        for (; it != endIt; ++it) {
            (*it).saveXML(context, document, element);
        }
    }

    mService.saveXML(context, &mBindings, document, element);
}
#endif

void Definitions::setWantedService(const QString &name)
{
    mWantedService = name;
}
