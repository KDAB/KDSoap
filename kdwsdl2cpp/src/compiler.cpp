/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
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
    : QObject(nullptr)
{
}

void Compiler::run()
{
    download();
}

void Compiler::download()
{
    FileProvider provider(Settings::self()->useLocalFilesOnly());

    QString fileName;
    if (provider.get(Settings::self()->wsdlUrl(), fileName)) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug("Unable to download file %s", Settings::self()->wsdlUrl().toEncoded().constData());
            provider.cleanUp();
            QCoreApplication::exit(1);
            return;
        }

        // qDebug() << "parsing" << fileName;
        QString errorMsg;
        int errorLine, errorCol;
        QDomDocument doc;
        if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorCol)) {
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
    definitions.setUseLocalFilesOnly(Settings::self()->useLocalFilesOnly());
    definitions.setWantedService(Settings::self()->wantedService());
    if (definitions.loadXML(&context, element)) {

        definitions.fixUpDefinitions(/*&context, element*/);


        WSDL wsdl;
        wsdl.setDefinitions(definitions);
        wsdl.setNamespaceManager(namespaceManager);

        KODE::Code::setDefaultIndentation(4);
        KWSDL::Converter converter;
        converter.setWSDL(wsdl);

        if (!converter.convert()) {
            QCoreApplication::exit(4);
        } else {
            KWSDL::Creator creator;
            creator.setOutputDirectory(Settings::self()->outputDirectory());
            creator.setSourceFile(Settings::self()->wsdlFileName());
            creator.setHeaderFileName(Settings::self()->headerFileName());
            creator.setImplementationFileName(Settings::self()->implementationFileName());

            creator.setClasses(converter.classes());
            if (Settings::self()->generateHeader()) {
                creator.createHeader();
            }
            if (Settings::self()->generateImplementation()) {
                creator.createImplementation();
            }

            QCoreApplication::exit(0);
        }
    } else {
        QCoreApplication::exit(3);
    }
}

#include "moc_compiler.cpp"
