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

#ifndef KWSDL_CREATOR_H
#define KWSDL_CREATOR_H

#include <code_generation/class.h>
#include <code_generation/file.h>
#include <code_generation/license.h>
#include <code_generation/printer.h>

namespace KWSDL
{

class Creator
{
public:
    Creator();

    void setOutputDirectory(const QString &outputDirectory);
    void setSourceFile(const QString &sourceFile);

    void setHeaderFileName(const QString &headerFileName);
    void setImplementationFileName(const QString &implementationFileName);
    void setClasses(const KODE::Class::List &list);

    void createHeader();
    void createImplementation();

private:
    KODE::File _file;
    KODE::Printer _printer;
};

}

#endif
