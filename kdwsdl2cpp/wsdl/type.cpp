/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>

#include <schema/parser.h>

#include "../src/settings.h"

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

static QMap<QUrl, QString> localSchemas()
{
    QMap<QUrl, QString> map;
    map.insert(QUrl(QLatin1String("http://schemas.xmlsoap.org/soap/encoding/")), QLatin1String(":/libkode/soapenc-1.1.xsd"));
    map.insert(QUrl(QLatin1String("http://www.w3.org/2003/05/soap-encoding")), QLatin1String(":/libkode/soapenc-1.2.xsd"));
    return map;
}

bool Type::loadXML(ParserContext *context, const QDomElement &element)
{
    XSD::Parser parser(context, nameSpace(), Settings::self()->useLocalFilesOnly(), Settings::self()->importPathList());
    parser.setLocalSchemas(localSchemas());
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        NSManager namespaceManager(context, child);
        if (namespaceManager.nameSpace(child) == XSD::Parser::schemaUri() && namespaceManager.localName(child) == QLatin1String("schema")) {
            // qDebug() << "Loading schema" << nameSpace();
            if (!parser.parseSchemaTag(context, child)) {
                return false;
            }
        }

        child = child.nextSiblingElement();
    }
    if (!parser.resolveForwardDeclarations()) {
        return false;
    }
    mTypes += parser.types();

    return true;
}

void Type::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    Q_UNUSED(context);
    Q_UNUSED(document);
    Q_UNUSED(parent);
}
