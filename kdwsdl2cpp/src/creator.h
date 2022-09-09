/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_CREATOR_H
#define KWSDL_CREATOR_H

#include <code_generation/class.h>
#include <code_generation/file.h>
#include <code_generation/license.h>
#include <code_generation/printer.h>

namespace KWSDL {

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
