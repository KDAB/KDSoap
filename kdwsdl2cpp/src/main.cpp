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

#include "compiler.h"
#include "settings.h"

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

static void showHelp(const char *appName)
{
    fprintf(stderr, "%s %s\n", WSDL2CPP_DESCRIPTION, WSDL2CPP_VERSION_STR);
    fprintf(stderr,
            "Usage:\n"
            "   Header file: %s [options] -o <headerfile> <wsdlfile>\n"
            "   Impl.  file: %s [options] -o <cppfile> -impl <headerfile> <wsdlfile>\n"
            "   Both files : %s [options] -both <basefile> <wsdlfile>\n"
            "\n"
            "Options:\n"
            "  -h, -help                 display this help and exit\n"
            "  -v, -version              display version\n"
            "  -s, -service              name of the service to generate\n"
            "  -o <file>                 output the generated file into <file>\n"
            "  -impl <headerfile>        generate the implementation(.cpp) file, and #include <headerfile>\n"
            "  -both <basefilename>      generate both the header(.h) and the implementation(.cpp) file\n"
            "  -server                   generate server-side base class, instead of client service\n"
            "  -exportMacro <macroname>  set the export declaration to use for generated classes\n"
            "  -namespace <ns>           put all generated classes into the given C++ namespace\n"
            "  -namespaceMapping <mapping>\n"
            "                            add the uri=code mapping\n"
            "                            if <mapping> begins with '@', read from file instead\n"
            "                            one entry per line\n"
            "                            (affects the generated class names)\n"
            "  -optional-element-type <type>\n"
            "                            use <type> as the getter return value for optional elements.\n"
            "                            <type> can be either raw-pointer, boost-optional or std-optional\n"
            "  -keep-unused-types        keep the wsdl unused types to the cpp generation step\n"
            "  -import-path <importpath> search for files first in this path before\n"
            "                            downloading them. may be specified multiple times.\n"
            "                            the file needs to be located at:\n"
            "                            <importpath>/<url-host>/<url-path>\n"
            "  -use-local-files-only     only use local files instead of downloading them\n"
            "                            automatically. this can be used to force the correct\n"
            "                            use of the import-path option\n"
            "  -help-on-missing          When groups or basic types could not be found, display\n"
            "                            available types (helps with wrong namespaces)\n"
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0) && !defined(QT_NO_SSL)
            "  -pkcs12file               Load a certificate from a PKCS12 file. You can use this option\n"
            "                            if the WSDL file (or files refering to it) is served from a \n"
            "                            location which require certificate based authentication\n"
            "  -pkcs12password           Pass the password for the certificate file if required.\n"
            "                            This option is not secure and should be used with caution\n"
            "                            if other users of the machine are capable to see the running\n"
            "                            processes ran by the current user.\n"
            "  -no-sync                  Do not generate synchronous API methods to the client code\n"
            "  -no-async                 Do not generate asynchronous API methods to the client code\n"
            "  -no-async-jobs            Do not generate asynchronous job API classes to the client code\n"
#endif
            "\n", appName, appName, appName);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    Q_INIT_RESOURCE(schemas);

    const char *fileName = nullptr;
    QFileInfo outputFile;
    bool both = false;
    bool impl = false;
    bool outfileGiven = false;
    bool server = false;
    QString headerFile;
    QString serviceName;
    QString exportMacro;
    QString nameSpace;
    Settings::NSMapping nsmapping; // XML mappings from URL to short code
    Settings::OptionalElementType optionalElementType = Settings::ENone;
    bool keepUnusedTypes = false;
    QStringList importPathList;
    bool useLocalFilesOnly = false;
    bool helpOnMissing = false;
    bool skipAsync = false, skipSync = false, skipAsyncJobs = false;
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0) && !defined(QT_NO_SSL)
    QString pkcs12File, pkcs12Password;
#endif

    int arg = 1;
    while (arg < argc) {
        QString opt = QString::fromLocal8Bit(argv[arg]);
        if (opt == QLatin1String("-h") || opt == QLatin1String("-help")) {
            showHelp(argv[0]);
            return 0;
        } else if (opt == QLatin1String("-impl")) {
            impl = true;
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            headerFile = QFile::decodeName(argv[arg]);
        } else if (opt == QLatin1String("-both")) {
            both = true;
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            outputFile.setFile(QFile::decodeName(argv[arg]));
        } else if (opt == QLatin1String("-server")) {
            server = true;
        } else if (opt == QLatin1String("-v") || opt == QLatin1String("-version")) {
            fprintf(stderr, "%s %s\n", WSDL2CPP_DESCRIPTION, WSDL2CPP_VERSION_STR);
            return 0;
        } else if (opt == QLatin1String("-o") || opt == QLatin1String("-output")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            outfileGiven = true;
            outputFile.setFile(QFile::decodeName(argv[arg]));
        } else if (opt == QLatin1String("-s") || opt == QLatin1String("-service")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            serviceName = QFile::decodeName(argv[arg]);
        } else if (opt == QLatin1String("-exportMacro")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            exportMacro = argv[arg];
        } else if (opt == QLatin1String("-namespace")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            nameSpace = argv[arg];
        } else if (opt == QLatin1String("-namespaceMapping")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            QString mapping = argv[arg];
            if (mapping.startsWith('@')) {
                QString mappingFileName = QFile::decodeName(argv[arg] + 1); // +1 to skip the '@'
                QFile file(mappingFileName);
                if (!file.open(QIODevice::ReadOnly)) {
                    fprintf(stderr, "Error reading %s: %s\n", QFile::encodeName(mappingFileName).constData(), qPrintable(file.errorString()));
                    showHelp(argv[0]);
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
                        nsmapping[uri] = target;
                    }
                }
            } else {
                QString uri = mapping.section("=", 0, -2);
                QString target = mapping.section("=", -1, -1);
                nsmapping[uri] = target;
            }
        } else if (opt == QLatin1String("-optional-element-type")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            QLatin1String optType(argv[arg]);
            if (optType == QLatin1String("raw-pointer")) {
                optionalElementType = Settings::ERawPointer;
            } else if (optType == QLatin1String("boost-optional")) {
                optionalElementType = Settings::EBoostOptional;
            } else if (optType == QLatin1String("std-optional")) {
                optionalElementType = Settings::EStdOptional;
            }
        } else if (opt == QLatin1String("-keep-unused-types")) {
            keepUnusedTypes = true;
        } else if (opt == QLatin1String("-import-path")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            importPathList.append(QFile::decodeName(argv[arg]));
        } else if (opt == QLatin1String("-use-local-files-only")) {
            useLocalFilesOnly = true;
        } else if (opt == QLatin1String("-help-on-missing")) {
            helpOnMissing = true;
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0) && !defined(QT_NO_SSL)
        } else if (opt == QLatin1String("-pkcs12file")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            pkcs12File = QLatin1String(argv[arg]);
        } else if (opt == QLatin1String("-pkcs12password")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            pkcs12Password = QLatin1String(argv[arg]);
#endif
        } else if (opt == QLatin1String("-no-sync")) {
            skipSync = true;
        } else if (opt == QLatin1String("-no-async")) {
            skipAsync = true;
        } else if (opt == QLatin1String("-no-async-jobs")) {
            skipAsyncJobs = true;
        } else if (!fileName) {
            fileName = argv[arg];
        } else {
            showHelp(argv[0]);
            return 1;
        }

        ++arg;
    }

    if (!fileName) {
        showHelp(argv[0]);
        return 1;
    }

    // if you're saying "just make the impl-file", you can't
    //    also say "make both the header and the impl"
    if (both && (outfileGiven || impl)) {
        showHelp(argv[0]);
        return 1;
    }


    if (both) {
        Settings::self()->setGenerateHeader(true);
        Settings::self()->setGenerateImplementation(true);
        Settings::self()->setHeaderFileName(outputFile.fileName() + ".h");
        Settings::self()->setImplementationFileName(outputFile.fileName() + ".cpp");
    } else if (impl) {
        Settings::self()->setGenerateHeader(false);
        Settings::self()->setGenerateImplementation(true);
        Settings::self()->setHeaderFileName(headerFile);
        Settings::self()->setImplementationFileName(outputFile.fileName());
    } else {
        Settings::self()->setGenerateHeader(true);
        Settings::self()->setGenerateImplementation(false);
        Settings::self()->setHeaderFileName(outputFile.fileName());
        Settings::self()->setImplementationFileName("UNUSED");
    }


    Settings::self()->setGenerateServerCode(server);
    Settings::self()->setOutputDirectory(outputFile.absolutePath());
    Settings::self()->setWsdlFile(fileName);
    Settings::self()->setWantedService(serviceName);
    Settings::self()->setExportDeclaration(exportMacro);
    Settings::self()->setNameSpace(nameSpace);
    Settings::self()->setNamespaceMapping(nsmapping);
    Settings::self()->setOptionalElementType(optionalElementType);
    Settings::self()->setKeepUnusedTypes(keepUnusedTypes);
    Settings::self()->setImportPathList(importPathList);
    Settings::self()->setUseLocalFilesOnly(useLocalFilesOnly);
    Settings::self()->setHelpOnMissing(helpOnMissing);
    Settings::self()->setSkipSync(skipSync);
    Settings::self()->setSkipAsync(skipAsync);
    Settings::self()->setSkipAsyncJobs(skipAsyncJobs);

    KWSDL::Compiler compiler;
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0) && !defined(QT_NO_SSL)
    if (!pkcs12File.isEmpty()) {
        QFile certFile(pkcs12File);
        if (certFile.open(QFile::ReadOnly)) {
            QSslKey key;
            QSslCertificate certificate;
            QList<QSslCertificate> caCertificates;
            const bool certificateLoaded = QSslCertificate::importPkcs12(&certFile,
                                                         &key,
                                                         &certificate,
                                                         &caCertificates,
                                                         pkcs12Password.toLocal8Bit());
            certFile.close();
            if (!certificateLoaded) {
                fprintf(stderr, "Unable to load the %s certificate file\n",
                        pkcs12File.toLocal8Bit().constData());
                if (!pkcs12Password.isEmpty())
                    fprintf(stderr, "Please make sure that you have passed the correct password\n");
                else
                    fprintf(stderr, "Maybe it is password protected?\n");
                return 1;
            }

            // set the loaded certificate info as default SSL config
            QSslConfiguration sslConfig;
            sslConfig.setPrivateKey(key);
            sslConfig.setLocalCertificate(certificate);
            sslConfig.setCaCertificates(caCertificates);
            QSslConfiguration::setDefaultConfiguration(sslConfig);
        } else {
            fprintf(stderr, "Failed to open the %s certificate file for reading\n",
                    pkcs12File.toLocal8Bit().constData());
            return 1;
        }
    }
#endif

    // so that we have an event loop, for downloads
    QTimer::singleShot(0, &compiler, SLOT(run()));

    return app.exec();
}
