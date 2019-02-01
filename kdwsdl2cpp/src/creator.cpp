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
    KODE::Code::setDefaultIndentation(4);

    // Set generated header details.
    _printer.setCreationWarning(true);
    _printer.setGenerator(QLatin1String("KDAB's kdwsdl2cpp"));

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
    KODE::Class::List::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        _file.insertClass(*it);
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
