/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include "import.h"

using namespace KWSDL;

Import::Import()
{
}

Import::Import(const QString &nameSpace)
    : Element(nameSpace)
{
}

Import::~Import()
{
}

void Import::setImportNamespace(const QString &nameSpace)
{
    mImportNamespace = nameSpace;
}

QString Import::importNamespace() const
{
    return mImportNamespace;
}

void Import::setLocation(const QUrl &location)
{
    mLocation = location;
}

QUrl Import::location() const
{
    return mLocation;
}

void Import::loadXML(ParserContext *context, const QDomElement &element)
{
    mImportNamespace = element.attribute(QLatin1String("namespace"));
    if (mImportNamespace.isEmpty()) {
        context->messageHandler()->warning(QLatin1String("Import: 'namespace' required"));
    }

    mLocation = QUrl(element.attribute(QLatin1String("schemaLocation")));
    if (!mLocation.isValid()) {
        context->messageHandler()->warning(QLatin1String("Import: 'schemaLocation' required"));
    }
}

void Import::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement(QLatin1String("import"));
    parent.appendChild(element);

    if (mImportNamespace.isEmpty()) {
        element.setAttribute(QLatin1String("namespace"), mImportNamespace);
    } else {
        context->messageHandler()->warning(QLatin1String("Import: 'namespace' required"));
    }

    if (mLocation.isValid()) {
        element.setAttribute(QLatin1String("schemaLocation"), mLocation.toString());
    } else {
        context->messageHandler()->warning(QLatin1String("Import: 'schemaLocation' required"));
    }
}
