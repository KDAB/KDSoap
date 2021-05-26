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

#include <code_generation/file.h>
#include <code_generation/license.h>
#include <code_generation/printer.h>

#include "settings.h"

#include "creator.h"

#include <QDebug>

using namespace KWSDL;

Creator::Creator()
{
    // Set generated header details.
    _printer.setCreationWarning(true);
    _printer.setGenerator(QLatin1String("KDAB's kdwsdl2cpp"));
    _printer.setStatementsAfterIncludes(QStringList() << "#undef daylight"
                                                      << "#undef timezone");

    // Qt-like coding style
    _printer.setLabelsDefineIndent(false);
    _printer.setIndentLabels(false);

    _file.setLicense(KODE::License::GeneratedNoRestriction);
}

void Creator::setOutputDirectory(const QString &outputDirectory)
{
    _printer.setOutputDirectory(outputDirectory);
}

void Creator::setSourceFile(const QString &sourceFile)
{
    _printer.setSourceFile(sourceFile);
}

void Creator::setHeaderFileName(const QString &headerFileName)
{
    _file.setHeaderFilename(headerFileName);
}

void Creator::setImplementationFileName(const QString &implementationFileName)
{
    _file.setImplementationFilename(implementationFileName);
}

void Creator::setClasses(const KODE::Class::List &list)
{
    for (const KODE::Class &newClass : qAsConst(list)) {
        _file.insertClass(newClass);
    }
}

void Creator::createHeader()
{
    _printer.printHeader(_file);
}

void Creator::createImplementation()
{
    _printer.printImplementation(_file);
}
