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

#include <libkode/file.h>
#include <libkode/license.h>
#include <libkode/printer.h>

#include "settings.h"

#include "creator.h"

#include <QDebug>

using namespace KWSDL;

Creator::Creator()
{
}

void Creator::create(const KODE::Class::List &classes)
{
    KODE::Printer printer;
    printer.setOutputDirectory(Settings::self()->outputDirectory());

    // Set generated header details.
    printer.setCreationWarning(true);
    printer.setGenerator(QLatin1String("KDAB's kdwsdl2cpp"));
    printer.setSourceFile(Settings::self()->wsdlFileName());

    // Qt-like coding style
    printer.setLabelsDefineIndent(false);
    printer.setIndentLabels(false);

    //qDebug() << "Create server=" << Settings::self()->generateServerCode() << "impl=" << Settings::self()->generateImplementation();

    KODE::File file;

    if (Settings::self()->generateImplementation()) {
        file.setImplementationFilename(Settings::self()->outputFileName());
        file.setHeaderFilename(Settings::self()->headerFile());
    } else {
        file.setHeaderFilename(Settings::self()->outputFileName());
    }

    file.setLicense(KODE::License::GeneratedNoRestriction);

    KODE::Class::List::ConstIterator it;
    for (it = classes.constBegin(); it != classes.constEnd(); ++it) {
        file.insertClass(*it);
    }

    if (Settings::self()->generateImplementation()) {
        printer.printImplementation(file);
    } else {
        printer.printHeader(file);
    }
}
