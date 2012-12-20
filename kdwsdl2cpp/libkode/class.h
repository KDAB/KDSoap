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
#ifndef KODE_CLASS_H
#define KODE_CLASS_H

#include <QtCore/QList>
#include <QtCore/QStringList>

#include "enum.h"
#include "function.h"
#include "membervariable.h"
#include "typedef.h"

#include <kode_export.h>

namespace KODE {

class ClassList;
/**
 * This class abstracts a class object with functions,
 * member variables etc.
 */
class KODE_EXPORT Class
{
  public:
    typedef ClassList List;

    /**
     * Creates a new class object.
     */
    Class();

    /**
     * Creates a new class object from @param other.
     */
    Class( const Class &other );

    /**
     * Creates a new class object with a given @param name.
     *
     * @param nameSpace The namespace the class object should be part of.
     */
    Class( const QString &name, const QString &nameSpace = QString() );

    /**
     * Destroys the class object.
     */
    ~Class();

    /**
     * Assignment operator.
     */
    Class& operator=( const Class &other );

    /**
     * Returns whether this class object is valid.
     */
    bool isValid() const;

    /**
     * Sets the name of the class object, possibly splitting out the namespace part of it.
     * E.g. Foo::Bar will set namespace = Bar, class name = Foo.
     *
     * This will treat anything except the last substring, as a namespace.
     * So this is not suited for nested classes. Use Class(A::B, NS) for nested classes.
     */
    void setNamespaceAndName( const QString& name );

    /**
     * Sets the @param name of the class object.
     */
    void setName( const QString &name );

    /**
     * Returns the name of the class object.
     */
    QString name() const;

    /**
     * Sets the namespace the class object should be part of.
     */
    void setNameSpace( const QString &nameSpace );

    /**
     * Returns the namespace the class object is part of.
     */
    QString nameSpace() const;

    /**
     * Returns the fully qualified class name including namespaces, e.g. NS1::NS2::ClassName
     */
    QString qualifiedName() const;

    /**
      Set export declaration with given name. This adds an include of a file
      name_export.h and a prefix to class declaration of NAME_EXPORT.
    */
    void setExportDeclaration( const QString &name );

    /**
      Return name of export declaration.
    */
    QString exportDeclaration() const;

    /**
     * Sets whether the class object shall use a d-pointer to store
     * its member variables.
     */
    void setUseDPointer( bool useDPointer, const QString& dPointer = "d" );

    /**
     * Returns whether the class object uses a d-pointer.
     */
    bool useDPointer() const;

    /**
     * Returns the name of the d pointer.
     * Usually d, but can be set to something else to avoid clashes with a d() method for instance.
     */
    QString dPointerName() const;

    /**
     * Sets whether the class object shall use a QSharedDataPointer d-pointer
     * and a private class that derives from QSharedData.
     * This is for implicitly-shared value classes (classes that can be copied).
     *
     * Setting this to true automatically sets canBeCopied to true and useDPointer to true.
     */
    void setUseSharedData( bool b, const QString& dPointer = "d" );

    /**
     * Returns whether the class object uses a QSharedDataPointer d-pointer.
     */
    bool useSharedData() const;

    /**
     * Sets whether the class can be copied (generates a copy constructor
     * and an operator= implementations, in case a d pointer is used).
     */
    void setCanBeCopied( bool b );

    /**
     * Returns whether the class instances can be copied.
     */
    bool canBeCopied() const;

    /**
     * Adds an include to the class object.
     *
     * @param file The include file like 'qfile.h' which will be
     *             printed as '#include <qfile.h>' in the header file.
     * @param forwardDeclaration The forward declaration like 'QFile'
     */
    void addInclude( const QString &file,
                     const QString &forwardDeclaration = QString() );

    /**
     * Adds several includes to the class object.
     *
     * @param files A list of include files like 'qfile.h'
     * @param forwardDeclaration A list of forward declarations like 'QFile'
     */
    void addIncludes( const QStringList &files,
                      const QStringList &forwardDeclarations = QStringList() );

    /**
     * Returns the list of includes.
     */
    QStringList includes() const;

    /**
     * Returns the list of forward declarations.
     */
    QStringList forwardDeclarations() const;

    /**
     * Adds a header include to the class object.
     *
     * @param file The header include file like 'qfile.h' which
     *             will be printed as '#include "qfile.h"' in the
     *             implementation.
     */
    void addHeaderInclude( const QString &file );

    /**
     * Adds a list of header includes to the class object.
     */
    void addHeaderIncludes( const QStringList &files );

    /**
     * Returns the list of header includes.
     */
    QStringList headerIncludes() const;

    /**
     * Adds a @param function to the class object.
     */
    void addFunction( const Function &function );

    /**
     * Returns the list of all functions.
     */
    Function::List functions() const;

    /**
     * Adds a member @param variable to the class object.
     */
    void addMemberVariable( const MemberVariable &variable );

    /**
     * Returns the list of all member variables.
     */
    MemberVariable::List memberVariables() const;

    /**
     * Adds a base class definition to the class object.
     *
     * @param baseClass A class object which describes the base class.
     */
    void addBaseClass( const Class &baseClass );

    /**
     * Returns the list of all base classes.
     */
    Class::List baseClasses() const;

    /**
     * Adds a typedef to the class object.
     */
    void addTypedef( const Typedef &typedefValue );

    /**
     * Returns the list of all typedefs.
     */
    Typedef::List typedefs() const;

    /**
     * Adds an enum to the class object.
     */
    void addEnum( const Enum &enumValue );

    /**
     * Returns the list of all enums.
     */
    Enum::List enums() const;

    /**
     * Returns true, if the enum with the given name already exists. Returns
     * false, if not.
     */
    bool hasEnum( const QString &name ) const;

    /**
     * Sets the @param documentation of the class object.
     */
    void setDocs( const QString &documentation );

    /**
     * Returns the documentation of the class object.
     */
    QString docs() const;

    /**
     * Returns whether the class object has a function with
     * the given @param name.
     */
    bool hasFunction( const QString &name ) const;

    /**
     * Returns whether the class object is a QObject.
     *
     * That's the case when one of its functions has the Signal
     * or Slot flag set.
     */
    bool isQObject() const;

    /**
     * Adds a nested class to this class.
     */
    void addNestedClass( const Class &nestedClass );

    /**
     * Return the list of all nested classes.
     */
    Class::List nestedClasses() const;

    /**
     * Return the name of the parent class name in a nested class.
     */
    QString parentClassName() const;
    /**
     * Set the name of the parent class in a nested class.
     */
    void setParentClassName( const QString &name );

    /**
     * Adds a declaration macro at the top of the class, like Q_PROPERTY(...)
     * or Q_INTERFACES(...).
     */
    void addDeclarationMacro( const QString& macro );

    /**
     * Returns the list of declaration macros added by addDeclarationMacro()
     */
    QStringList declarationMacros() const;

  private:
    class Private;
    Private* d;
};

class ClassList: public QList<Class>
{
public:
    /**
     * Sort the classes so that the result compiles, i.e. so that a class using another
     * (via member vars or via inheritance) is after it in the list.
     *
     * @param excludedClasses list of base classes which can be excluded from the search
     * for dependencies, usually because them come from underlying libraries.
     * All classes starting with Q are automatically excluded
     */
    void sortByDependencies( const QStringList& excludedClasses = QStringList() );
    // maybe we could have a bool ignoreUnknownClasses, too, for people who write bugfree code...

    void addClass(const Class& cl);

    QStringList classNames() const;

    iterator findClass(const QString& name);
};

}

#endif
