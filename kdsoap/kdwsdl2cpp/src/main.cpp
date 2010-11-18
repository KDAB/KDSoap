/****************************************************************************
** Copyright (C) 2010-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

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
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

static const char* WSDL2CPP_DESCRIPTION = "KDAB's WSDL to C++ compiler";
static const char* WSDL2CPP_VERSION_STR = "1.0";

static void showHelp(const char *appName)
{
    fprintf(stderr, "%s %s\n", WSDL2CPP_DESCRIPTION, WSDL2CPP_VERSION_STR);
    fprintf(stderr, "Usage: %s [options] [-impl <headerfile>] <wsdlfile>\n\n"
            "  -h, -help                 display this help and exit\n"
            "  -v, -version              display version\n"
            "  -s, -service              name of the service to generate\n"
            "  -o <file>                 place the output into <file>\n"
            "  -impl <headerfile>        generate the implementation file, and #include <headerfile>\n"
            "\n", appName);
}

int main( int argc, char **argv )
{
    const char *fileName = 0;
    QString outputFile;
    bool impl = false;
    QString headerFile;
    QString serviceName;

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
        } else if (opt == QLatin1String("-s") || opt == QLatin1String("-service")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            serviceName = QFile::decodeName(argv[arg]);
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

    QCoreApplication app( argc, argv );

    Settings::self()->setGenerateImplementation(impl, headerFile);
    Settings::self()->setOutputFileName(outputFile);
    Settings::self()->setWsdlFile(fileName);
    Settings::self()->setWantedService(serviceName);

    KWSDL::Compiler compiler;

    // so that we have an event loop, for downloads
    QTimer::singleShot( 0, &compiler, SLOT( run() ) );

    return app.exec();
}
