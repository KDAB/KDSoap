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

    mLocation = element.attribute(QLatin1String("schemaLocation"));
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
