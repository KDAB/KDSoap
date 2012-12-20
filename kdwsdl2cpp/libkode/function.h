/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009 David Faure <dfaure@kdab.com>

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
#ifndef KODE_FUNCTION_H
#define KODE_FUNCTION_H

#include <QtCore/QStringList>

#include "code.h"

#include <kode_export.h>

namespace KODE {

/**
 * This class represents a function.
 */
class KODE_EXPORT Function
{
  public:
    class Argument {
      public:
        typedef QList<Argument> List;

        Argument( const QString &declaration,
          const QString &defaultArgument = QString() );

        QString headerDeclaration() const;
        QString bodyDeclaration() const;

      private:
        class ArgumentPrivate;
        ArgumentPrivate *d;
    };

    typedef QList<Function> List;

    /**
     * The different access specifiers.
     *
     * @li Public     - Public access
     * @li Protected  - Protected access
     * @li Private    - Private access
     * @li Signal     - Qt Signal
     * @li Slot       - Qt Slot
     */
    enum AccessSpecifier { Public = 1, Protected = 2, Private = 4, Signal = 8, Slot = 16 };

    /**
     * Creates a new function.
     */
    Function();

    /**
     * Creates a new function from @param other.
     */
    Function( const Function &other );

    /**
     * Creates a new function with the given @param name.
     *
     * @param returnType The return type.
     * @param access The access type (@see AccessSpecifier).
     * @param isStatic If true, the function is marked as static.
     */
    Function( const QString &name, const QString &returnType = QString(),
              int access = Public, bool isStatic = false );

    /**
     * Destroys the function.
     */
    ~Function();

    /**
     * Assignment operator.
     */
    Function& operator=( const Function &other );

    /**
     * Sets the @param name of the function.
     */
    void setName( const QString &name );

    /**
     * Returns the name of the function.
     */
    QString name() const;

    /**
     * Sets the return type of the function.
     */
    void setReturnType( const QString &returnType );

    /**
     * Returns the return type of the function.
     */
    QString returnType() const;

    /**
     * Sets whether the function is marked as const.
     */
    void setConst( bool isConst );

    /**
     * Returns whether the function is marked as const.
     */
    bool isConst() const;

    /**
     * Sets whether the function is marked as static.
     */
    void setStatic( bool isStatic );

    /**
     * Returns whether the function is marked as static.
     */
    bool isStatic() const;

    enum VirtualMode { NotVirtual, Virtual, PureVirtual };
    /**
     * Sets whether the function is marked as virtual or pure virtual.
     */
    void setVirtualMode( VirtualMode v );

    /**
     * Returns whether the function is marked as virtual or pure virtual.
     */
    VirtualMode virtualMode() const;

    /**
     * Adds an @param argument to the function.
     */
    void addArgument( const Function::Argument &argument );

    /**
     * Adds an @param argument to the function.
     */
    void addArgument( const QString &argument );

    /**
     * Sets the complete argument string of the function.
     * This method does not support default values currently.
     */
    void setArgumentString( const QString &argumentString );

    /**
     * Returns the list of all arguments.
     * @param forImplementation if true, default values are omitted
     */
    Argument::List arguments() const;

    /**
     * @return whether the function has any arguments
     */
    bool hasArguments() const;

    /**
     * Adds an initializer to the function.
     */
    void addInitializer( const QString &initializer );

    /**
     * Returns the list of all initializers.
     */
    QStringList initializers() const;

    /**
     * Sets the @param body code of the function.
     */
    void setBody( const QString &body );

    /**
     * Sets the @param body code of the function.
     */
    void setBody( const Code &body );

    /**
     * Adds a @param line to the body code of the function.
     */
    void addBodyLine( const QString &line );

    /**
     * Returns the body code of the function.
     */
    QString body() const;

    /**
     * Sets the access @param specifier of the function.
     */
    void setAccess( int specifier );

    /**
     * Returns the access specifier of the function.
     */
    int access() const;

    /**
     * Returns access specifier of the function as string.
     */
    QString accessAsString() const;

    /**
     * Sets the @param documentation of the function.
     */
    void setDocs( const QString &documentation );

    /**
     * Returns the documentation of the function.
     */
    QString docs() const;

  private:
    class FunctionPrivate;
    FunctionPrivate *d;
};

}

QDebug operator<<(QDebug dbg, const KODE::Function &func);

#endif
