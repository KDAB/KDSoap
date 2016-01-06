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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

static const char *WSDL2CPP_DESCRIPTION = "KDAB's WSDL to C++ compiler";
static const char *WSDL2CPP_VERSION_STR = "2.0";

static void showHelp(const char *appName)
{
    fprintf(stderr, "%s %s\n", WSDL2CPP_DESCRIPTION, WSDL2CPP_VERSION_STR);
    fprintf(stderr, "Usage: %s [options] [-impl <headerfile>] <wsdlfile>\n\n"
            "  -h, -help                 display this help and exit\n"
            "  -v, -version              display version\n"
            "  -s, -service              name of the service to generate\n"
            "  -o <file>                 generate the header file into <file>\n"
            "  -impl <headerfile>        generate the implementation file, and #include <headerfile>\n"
            "  -server                   generate server-side base class, instead of client service\n"
            "  -exportMacro <macroname>  set the export declaration to use for generated classes\n"
            "  -namespace <ns>           put all generated classes into the given C++ namespace\n"
            "  -optional-element-type <type>\n"
            "                            use <type> as the getter return value for optional elements.\n"
            "                            <type> can be either raw-pointer or boost-optional\n"
            "\n", appName);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const char *fileName = 0;
    QFileInfo outputFile;
    bool impl = false;
    bool server = false;
    QString headerFile;
    QString serviceName;
    QString exportMacro;
    QString nameSpace;
    Settings::OptionalElementType optionalElementType = Settings::ENone;

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
            }
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

    Settings::self()->setGenerateServerCode(server);
    Settings::self()->setGenerateImplementation(impl, headerFile);
    Settings::self()->setOutputFileName(outputFile.fileName());
    Settings::self()->setOutputDirectory(outputFile.absolutePath());
    Settings::self()->setWsdlFile(fileName);
    Settings::self()->setWantedService(serviceName);
    Settings::self()->setExportDeclaration(exportMacro);
    Settings::self()->setNameSpace(nameSpace);
    Settings::self()->setOptionalElementType(optionalElementType);
    KWSDL::Compiler compiler;

    // so that we have an event loop, for downloads
    QTimer::singleShot(0, &compiler, SLOT(run()));

    return app.exec();
}
