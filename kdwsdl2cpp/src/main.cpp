/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include "compiler.h"
#include "settings.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QSslCertificate>
#include <QSslKey>
#include <QTimer>

static const char *WSDL2CPP_DESCRIPTION = "KDAB's WSDL to C++ compiler";
static const char *WSDL2CPP_VERSION_STR = "2.1";

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(WSDL2CPP_DESCRIPTION);
    QCoreApplication::setApplicationVersion(WSDL2CPP_VERSION_STR);

    Q_INIT_RESOURCE(schemas);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral(
                                         "%1\n\n"
                                         "   Header file: %2 [options] -o <headerfile> <wsdlfile>\n"
                                         "   Impl.  file: %2 [options] -o <cppfile> -impl <headerfile> <wsdlfile>\n"
                                         "   Both files : %2 [options] -both <basefile> <wsdlfile>")
                                         .arg(WSDL2CPP_DESCRIPTION, QCoreApplication::applicationFilePath()));
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.addVersionOption();

    struct
    {
        QCommandLineOption serviceName {{"s", "service"}, "Name of the service to generate.", "service"};
        QCommandLineOption outputFile {"o", "Output the generated file into <file>\n", "file"};
        QCommandLineOption impl {"impl", "Generate the implementation(.cpp) file, and #include <headerfile>.", "headerfile"};
        QCommandLineOption both {"both", "Generate both the header(.h) and the implementation(.cpp) file.", "basefilename"};
        QCommandLineOption server {"server", "Generate server-side base class, instead of client service."};
        QCommandLineOption exportMacro {"exportMacro", "Set the export declaration to use for generated classes.", "macroname"};
        QCommandLineOption namespaceOption {"namespace", "Put all generated classes into the given C++ namespace.", "ns"};
        QCommandLineOption namespaceMapping {"namespaceMapping", "Add the uri=code mapping\nif <mapping> begins with '@', read from file\n"
                                                                 "instead - one entry per line\n"
                                                                 "(affects the generated class names).",
                                             "mapping"};
        QCommandLineOption optionalElementType {"optional-element-type", "Use <type> as the getter return value for optional elements.\n"
                                                                         "<type> can be either raw-pointer, boost-optional or std-optional.",
                                                "type"};
        QCommandLineOption keepUnusedTypes {"keep-unused-types", "Keep the wsdl unused types to the cpp generation step."};
        QCommandLineOption importPath {"import-path", "Search for files first in this path before downloading them. may be specified multiple times.\n"
                                                      "The file needs to be located at:\n"
                                                      "<importpath>/<url-host>/<url-path>",
                                       "importpath"};
        QCommandLineOption useLocalFilesOnly {"use-local-files-only", "Only use local files instead of downloading them automatically.\n"
                                                                      "This can be used to force the correct use of the import-path option."};
        QCommandLineOption helpOnMissing {"help-on-missing", "When groups or basic types could not be found, display available types (helps with wrong namespaces)."};
#if !defined(QT_NO_SSL)
        QCommandLineOption pkcs12File {"pkcs12file", "Load a certificate from a PKCS12 file.\n"
                                                     "You can use this option if the WSDL file (or files referring to it) is served from a location which requires certificate based authentication.",
                                       "pkcs12file"};
        QCommandLineOption pkcs12Password {"pkcs12password", "Pass the password for the certificate file if required.\n"
                                                             "This option is not secure and should be used with caution if other users of the machine are capable to see the running processes ran by the current user.",
                                           "pkcs12password"};
#endif
        QCommandLineOption noSync {"no-sync", "Do not generate synchronous API methods to the client code."};
        QCommandLineOption noAsync {"no-async", "Do not generate asynchronous API methods to the client code."};
        QCommandLineOption noAsyncJobs {"no-async-jobs", "Do not generate asynchronous job API classes to the client code."};
    } options;

    parser.addOptions({options.serviceName,
                       options.outputFile,
                       options.impl,
                       options.both,
                       options.server,
                       options.exportMacro,
                       options.namespaceOption,
                       options.namespaceMapping,
                       options.optionalElementType,
                       options.keepUnusedTypes,
                       options.importPath,
                       options.useLocalFilesOnly,
                       options.helpOnMissing,
#if !defined(QT_NO_SSL)
                       options.pkcs12File,
                       options.pkcs12Password,
#endif
                       options.noSync,
                       options.noAsync,
                       options.noAsyncJobs});

    parser.addPositionalArgument("<wsdlfile>", "WSDL file to be processed.");

    parser.process(app);

    QFileInfo outputFile;
    Settings::NSMapping nsmapping; // XML mappings from URL to short code
    Settings::OptionalElementType optionalElementType = Settings::ENone;

    for (const QString &mapping : parser.values(options.namespaceMapping)) {
        if (mapping.startsWith('@')) {
            QString mappingFileName = mapping.mid(1); // +1 to skip the '@'
            QFile file(mappingFileName);
            if (!file.open(QIODevice::ReadOnly)) {
                fprintf(stderr, "Error reading %s: %s\n", QFile::encodeName(mappingFileName).constData(), qPrintable(file.errorString()));
                parser.showHelp(1);
                return 1;
            }

            while (!file.atEnd()) {
                QString mapping = file.readLine().trimmed();
                if (mapping.startsWith('#')) {
                    continue;
                }

                QString uri = mapping.section("=", 0, -2);
                QString target = mapping.section("=", -1, -1);
                if (!uri.isEmpty() && !target.isEmpty()) {
                    nsmapping[uri] = std::move(target);
                }
            }
        } else {
            QString uri = mapping.section("=", 0, -2);
            QString target = mapping.section("=", -1, -1);
            nsmapping[uri] = std::move(target);
        }
    }

    if (parser.isSet(options.optionalElementType)) {
        const QString optType = parser.value(options.optionalElementType);
        if (optType == QLatin1String("raw-pointer")) {
            optionalElementType = Settings::ERawPointer;
        } else if (optType == QLatin1String("boost-optional")) {
            optionalElementType = Settings::EBoostOptional;
        } else if (optType == QLatin1String("std-optional")) {
            optionalElementType = Settings::EStdOptional;
        }
    }

    if (parser.positionalArguments().size() != 1) {
        parser.showHelp(1);
        return 1;
    }

    // if you're saying "just make the impl-file", you can't
    //    also say "make both the header and the impl"
    if (parser.isSet(options.both) && (parser.isSet(options.outputFile) || parser.isSet(options.impl))) {
        parser.showHelp(1);
        return 1;
    }

    if (parser.isSet(options.both)) {
        outputFile.setFile(parser.value(options.both));
        Settings::self()->setGenerateHeader(true);
        Settings::self()->setGenerateImplementation(true);
        Settings::self()->setHeaderFileName(outputFile.fileName() + ".h");
        Settings::self()->setImplementationFileName(outputFile.fileName() + ".cpp");
    } else if (parser.isSet(options.impl)) {
        outputFile.setFile(parser.value(options.outputFile));
        Settings::self()->setGenerateHeader(false);
        Settings::self()->setGenerateImplementation(true);
        Settings::self()->setHeaderFileName(parser.value(options.impl));
        Settings::self()->setImplementationFileName(outputFile.fileName());
    } else {
        outputFile.setFile(parser.value(options.outputFile));
        Settings::self()->setGenerateHeader(true);
        Settings::self()->setGenerateImplementation(false);
        Settings::self()->setHeaderFileName(outputFile.fileName());
        Settings::self()->setImplementationFileName("UNUSED");
    }

    Settings::self()->setGenerateServerCode(parser.isSet(options.server));
    Settings::self()->setOutputDirectory(outputFile.absolutePath());
    Settings::self()->setWsdlFile(parser.positionalArguments().at(0));
    Settings::self()->setWantedService(parser.value(options.serviceName));
    Settings::self()->setExportDeclaration(parser.value(options.exportMacro));
    Settings::self()->setNameSpace(parser.value(options.namespaceOption));
    Settings::self()->setNamespaceMapping(nsmapping);
    Settings::self()->setOptionalElementType(optionalElementType);
    Settings::self()->setKeepUnusedTypes(parser.isSet(options.keepUnusedTypes));
    Settings::self()->setImportPathList(parser.values(options.importPath));
    Settings::self()->setUseLocalFilesOnly(parser.isSet(options.useLocalFilesOnly));
    Settings::self()->setHelpOnMissing(parser.isSet(options.helpOnMissing));
    Settings::self()->setSkipSync(parser.isSet(options.noSync));
    Settings::self()->setSkipAsync(parser.isSet(options.noAsync));
    Settings::self()->setSkipAsyncJobs(parser.isSet(options.noAsyncJobs));

    KWSDL::Compiler compiler;
#if !defined(QT_NO_SSL)
    const QString pkcs12File = parser.value(options.pkcs12File),
                  pkcs12Password = parser.value(options.pkcs12Password);
    if (!pkcs12File.isEmpty()) {
        QFile certFile(pkcs12File);
        if (certFile.open(QFile::ReadOnly)) {
            QSslKey key;
            QSslCertificate certificate;
            QList<QSslCertificate> caCertificates;
            const bool certificateLoaded =
                QSslCertificate::importPkcs12(&certFile, &key, &certificate, &caCertificates, pkcs12Password.toLocal8Bit());
            certFile.close();
            if (!certificateLoaded) {
                fprintf(stderr, "Unable to load the %s certificate file\n", pkcs12File.toLocal8Bit().constData());
                if (!pkcs12Password.isEmpty())
                    fprintf(stderr, "Please make sure that you have passed the correct password\n");
                else
                    fprintf(stderr, "Maybe it is password protected?\n");
                return 1;
            }

            // set the loaded certificate info as default SSL config
            QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
            sslConfig.setPrivateKey(key);
            sslConfig.setLocalCertificate(certificate);
            sslConfig.setCaCertificates(caCertificates);
            QSslConfiguration::setDefaultConfiguration(sslConfig);
        } else {
            fprintf(stderr, "Failed to open the %s certificate file for reading\n", pkcs12File.toLocal8Bit().constData());
            return 1;
        }
    }
#endif

    // so that we have an event loop, for downloads
    QTimer::singleShot(0, &compiler, &KWSDL::Compiler::run);

    return QCoreApplication::exec();
}
