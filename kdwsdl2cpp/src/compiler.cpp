/*
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

#include <QCoreApplication>
#include <QFile>
#include <QDebug>

#include <wsdl/wsdl.h>

#include <common/fileprovider.h>
#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include "converter.h"
#include "creator.h"
#include "settings.h"

#include "compiler.h"

using namespace KWSDL;

Compiler::Compiler()
    : QObject(0)
{
}

void Compiler::run()
{
    download();
}

void Compiler::download()
{
    FileProvider provider;

    QString fileName;
    if (provider.get(Settings::self()->wsdlUrl(), fileName)) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug("Unable to download file %s", Settings::self()->wsdlUrl().toEncoded().constData());
            provider.cleanUp();
            QCoreApplication::exit(1);
            return;
        }

        //qDebug() << "parsing" << fileName;
        QXmlInputSource source(&file);
        QXmlSimpleReader reader;
        reader.setFeature(QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true);

        QString errorMsg;
        int errorLine, errorCol;
        QDomDocument doc;
        if (!doc.setContent(&source, &reader, &errorMsg, &errorLine, &errorCol)) {
            qDebug("%s at (%d,%d)", qPrintable(errorMsg), errorLine, errorCol);
            QCoreApplication::exit(2);
            return;
        }

        parse(doc.documentElement());

        provider.cleanUp();
    } else {
        qDebug("Unable to download file %s", Settings::self()->wsdlUrl().toEncoded().constData());
        provider.cleanUp();
        QCoreApplication::exit(1);
        return;
    }
}

void Compiler::parse(const QDomElement &element)
{
    NSManager namespaceManager;

    // we don't parse xml.xsd, so hardcode the xml prefix (for xml:lang)
    namespaceManager.setPrefix(QLatin1String("xml"), NSManager::xmlNamespace());

    MessageHandler messageHandler;
    ParserContext context;
    context.setNamespaceManager(&namespaceManager);
    context.setMessageHandler(&messageHandler);
    context.setDocumentBaseUrl(QUrl(Settings::self()->wsdlBaseUrl()));

    Definitions definitions;
    definitions.setWantedService(Settings::self()->wantedService());
    if (definitions.loadXML(&context, element)) {

        definitions.fixUpDefinitions(/*&context, element*/);

        KODE::Code::setDefaultIndentation(4);

        WSDL wsdl;
        wsdl.setDefinitions(definitions);
        wsdl.setNamespaceManager(namespaceManager);

        KWSDL::Converter converter;
        converter.setWSDL(wsdl);

        if (!converter.convert()) {
            QCoreApplication::exit(4);
        } else {
            KWSDL::Creator creator;
            creator.create(converter.classes());
            QCoreApplication::exit(0);
        }
    } else {
        QCoreApplication::exit(3);
    }
}

#include "moc_compiler.cpp"
