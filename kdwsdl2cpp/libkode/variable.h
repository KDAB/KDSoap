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
#ifndef KODE_VARIABLE_H
#define KODE_VARIABLE_H

#include <QtCore/QStringList>

#include <kode_export.h>

namespace KODE {

/**
 * This class represents a variable.
 */
class KODE_EXPORT Variable
{
  public:
    typedef QList<Variable> List;

    /**
     * Creates a new variable.
     */
    Variable();

    /**
     * Creates a new variable from @param other.
     */
    Variable( const Variable &other );

    /**
     * Creates a new variable of the given @param type and with the
     * given @param name.
     *
     * @param isStatic If true the variable is marked as static.
     */
    Variable( const QString &name, const QString &type, bool isStatic = false );

    /**
     * Destroys the variable.
     */
    virtual ~Variable();

    /**
     * Assignment operator.
     */
    Variable& operator=( const Variable &other );

    /**
     * Sets the @param type of the variable.
     */
    void setType( const QString &type );

    /**
     * Returns the type of the variable.
     */
    QString type() const;

    /**
     * Sets the @param name of the variable.
     */
    void setName( const QString &name );

    /**
     * Returns the name of the variable.
     */
    QString name() const;

    /**
     * Sets whether the variable is static.
     */
    void setStatic( bool isStatic );

    /**
     * Returns whether the variable is static.
     */
    bool isStatic() const;

    /**
     * Sets the initializer of the variable.
     */
    void setInitializer( const QString &initializer );

    /**
     * Returns the initializer of the variable.
     */
    QString initializer() const;

  private:
    class Private;
    Private *d;
};

}

#endif
