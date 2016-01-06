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
#include <common/nsmanager.h>
#include <common/parsercontext.h>

#include <schema/parser.h>

#include <QDebug>

#include "type.h"

using namespace KWSDL;

Type::Type()
{
}

Type::Type(const QString &nameSpace)
    : Element(nameSpace)
{
}

Type::~Type()
{
}

void Type::setTypes(const XSD::Types &types)
{
    mTypes = types;
}

XSD::Types Type::types() const
{
    return mTypes;
}

bool Type::loadXML(ParserContext *context, const QDomElement &element)
{
    XSD::Parser parser(context, nameSpace());
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        if (namespaceManager.nameSpace(child) == XSD::Parser::schemaUri() &&
                namespaceManager.localName(child) == QLatin1String("schema")) {
            //qDebug() << "Loading schema" << nameSpace();
            if (!parser.parseSchemaTag(context, child)) {
                return false;
            }

            mTypes += parser.types();
        }

        child = child.nextSiblingElement();
    }
    return true;
}

void Type::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    Q_UNUSED(context);
    Q_UNUSED(document);
    Q_UNUSED(parent);
}
