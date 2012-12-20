/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KODE_FILE_H
#define KODE_FILE_H

#include "class.h"
#include "code.h"
#include "license.h"
#include "variable.h"

#include <kode_export.h>

namespace KODE {

/**
 * This class represents a file.
 */
class KODE_EXPORT File
{
  public:
    /**
     * Creates a new file.
     */
    File();

    /**
     * Creates a new file from @param other.
     */
    File( const File &other );

    /**
     * Destroys the file.
     */
    ~File();

    /**
     * Assignment operator.
     */
    File& operator=( const File &other );

    /**
     * Sets the filenames of both the .h and .cpp file.
     * The extensions will be automatically added.
     */
    void setFilename( const QString& baseName );

    /**
     * Sets the @param filename of the cpp file.
     */
    void setImplementationFilename( const QString &filename );

    /**
     * Sets the @param filename of the header file.
     */
    void setHeaderFilename( const QString &filename );

    /**
      Return filename of header file.
    */
    QString filenameHeader() const;

    /**
      Return filename of implementation file.
    */
    QString filenameImplementation() const;
    
    /**
     * Sets the name space of the file.
     */
    void setNameSpace( const QString &nameSpace );

    /**
     * Returns the name space of the file.
     */
    QString nameSpace() const;

    /**
     * Sets the @param project name of the file.
     */
    void setProject( const QString &project );

    /**
     * Returns the project name of the file.
     */
    QString project() const;

    /**
     * Add copyright statement to the file.
     *
     * @param year The year of participation.
     * @param name The name of the author.
     * @param email The email address of the author.
     */
    void addCopyright( int year, const QString &name, const QString &email );

    /**
     * Returns the list of all copyright statements.
     */
    QStringList copyrightStrings() const;

    /**
     * Sets the @param license of the file.
     */
    void setLicense( const License &license );

    /**
     * Returns the license of the file.
     */
    License license() const;

    /**
     * Adds an include to the file.
     */
    void addInclude( const QString &include );

    /**
     * Returns the list of all includes.
     */
    QStringList includes() const;

    /**
     * Inserts a class to the file.
     */
    void insertClass( const Class &newClass );

    /**
     * Returns a list of all classes.
     */
    Class::List classes() const;

    /**
     * Returns whether the file contains a class
     * with the given @param name.
     */
    bool hasClass( const QString &name );

    /**
     * Returns the class with the given @param name.
     */
    Class findClass( const QString &name );

    /**
     * Removes all classes from the file.
     */
    void clearClasses();

    /**
     * Removes all file functions from the file.
     */
    void clearFileFunctions();

    /**
     * Removes all file variables from the file.
     */
    void clearFileVariables();

    /**
     * Removes all file code from the file.
     */
    void clearCode();

    /**
     * Adds a file @param variable to the file.
     */
    void addFileVariable( const Variable &variable );

    /**
     * Returns the list of all file variables.
     */
    Variable::List fileVariables() const;

    /**
     * Adds a file @param function to the file.
     */
    void addFileFunction( const Function &function );

    /**
     * Returns the list of all file functions.
     */
    Function::List fileFunctions() const;

    /**
     * Adds a file enum to the file.
     */
    void addFileEnum( const Enum &enumValue );

    /**
     * Returns the list of all file enums.
     */
    Enum::List fileEnums() const;

    /**
     * Adds an external C declaration to the file.
     */
    void addExternCDeclaration( const QString &externalCDeclaration );

    /**
     * Returns the list of all external C declarations.
     */
    QStringList externCDeclarations() const;

    /**
     * Adds a file @param code block to the file.
     */
    void addFileCode( const Code &code );

    /**
     * Returns the file code block.
     */
    Code fileCode() const;

  private:
    class Private;
    Private *d;
};

}

#endif
