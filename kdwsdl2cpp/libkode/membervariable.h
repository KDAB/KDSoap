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
#ifndef KODE_MEMBERVARIABLE_H
#define KODE_MEMBERVARIABLE_H

#include <QtCore/QList>
#include <QtCore/QString>

#include "variable.h"

#include <kode_export.h>

namespace KODE {

/**
 * This class represents a member variable.
 *
 * It's basically a @see Variable with a
 * special formatted name.
 */
class KODE_EXPORT MemberVariable : public Variable
{
  public:
    typedef QList<MemberVariable> List;

    /**
     * Creates a new member variable.
     */
    MemberVariable();

    /**
     * Creates a new member variable from @param other.
     */
    MemberVariable( const MemberVariable &other );

    /**
     * Creates a new member variable of the given @param type
     * and with the given @param name.
     *
     * @param isStatic If true the variable is marked as static.
     */
    MemberVariable( const QString &name, const QString &type,
                    bool isStatic = false );

    /**
     * Destroys the member variable.
     */
    ~MemberVariable();

    /**
     * Assignment operator.
     */
    MemberVariable& operator=( const MemberVariable &other );

  private:
    class Private;
    Private *d;
};

}

#endif
