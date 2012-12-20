/*
    This file is part of KDE.

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
#ifndef KODE_STATEMACHINE_H
#define KODE_STATEMACHINE_H

#include "code.h"

#include <kode_export.h>

namespace KODE {

/**
 * This class represents a state machine.
 *
 * It can be used to create complex state machines
 * in an easy way.
 */
class KODE_EXPORT StateMachine
{
  public:
    /**
     * Creates a new state machine.
     */
    StateMachine();

    /**
     * Creates a new state machine from @param other.
     */
    StateMachine( const StateMachine &other );

    /**
     * Destroys the state machine.
     */
    ~StateMachine();

    /**
     * Assignment operator.
     */
    StateMachine& operator=( const StateMachine &other );

    /**
     * Sets the @param code for a special @param state.
     */
    void setState( const QString &state, const Code &code );

    /**
     * Sets the initial @param state, which is used when the
     * machine is started.
     */
    void setInitialState( const QString &state );

    /**
     * Returns the code for the state definitions.
     */
    Code stateDefinition() const;

    /**
     * Returns the code for the transition logic.
     */
    Code transitionLogic() const;

  private:
    class Private;
    Private *d;
};

}

#endif
