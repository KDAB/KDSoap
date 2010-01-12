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

#include "compiler.h"
#include "settings.h"

#include <QDir>
#include <QFile>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

static const char* WSDL2CPP_DESCRIPTION = "KDAB's WSDL to C++ compiler";
static const char* WSDL2CPP_VERSION_STR = "1.0";

static void showHelp(const char *appName)
{
    fprintf(stderr, "%s %s\n", WSDL2CPP_DESCRIPTION, WSDL2CPP_VERSION_STR);
    fprintf(stderr, "Usage: %s [options] [-impl] <wsdlfile>\n\n"
            "  -h, -help                 display this help and exit\n"
            "  -v, -version              display version\n"
            "  -d, -dependencies         display the dependencies\n"
            "  -o <file>                 place the output into <file>\n"
            "  -impl                     generate the implementation file\n"
            "\n", appName);
}

int main( int argc, char **argv )
{
    const char *fileName = 0;
    QString outputFile;
    bool dependencies = false;
    bool impl = false;

    int arg = 1;
    while (arg < argc) {
        QString opt = QString::fromLocal8Bit(argv[arg]);
        if (opt == QLatin1String("-h") || opt == QLatin1String("-help")) {
            showHelp(argv[0]);
            return 0;
        } else if (opt == QLatin1String("-d") || opt == QLatin1String("-dependencies")) {
            dependencies = true;
        } else if (opt == QLatin1String("-impl")) {
            impl = true;
        } else if (opt == QLatin1String("-v") || opt == QLatin1String("-version")) {
            fprintf(stderr, "%s %s\n", WSDL2CPP_DESCRIPTION, WSDL2CPP_VERSION_STR);
            return 0;
        } else if (opt == QLatin1String("-o") || opt == QLatin1String("-output")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            outputFile = QFile::decodeName(argv[arg]);
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

    if (dependencies) {
        // TODO output dependencies
        return 0;
    }

    QCoreApplication app( argc, argv );

    Settings::self()->setGenerateImplementation(impl);
    Settings::self()->setOutputFileName(outputFile);
    Settings::self()->setWsdlFile(fileName);

    KWSDL::Compiler compiler;

    // so that we have an event loop, for downloads
    QTimer::singleShot( 0, &compiler, SLOT( run() ) );

    return app.exec();
}
