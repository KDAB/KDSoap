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
#ifndef KODE_CODE_H
#define KODE_CODE_H

#include <kode_export.h>
#include <QtCore/QString>

namespace KODE {

/**
 * This class encapsulates a code block.
 */
class KODE_EXPORT Code
{
  public:
    /**
     * Creates a new code block.
     */
    Code();

    /**
     * Creates a new code block from @param other.
     */
    Code( const Code &other );

    /**
     * Creates a new code block with the given @param indent.
     */
    Code( int indent );

    /**
     * Destroys the code block.
     */
    ~Code();

    /**
     * Assignment operator.
     */
    Code& operator=( const Code &other );

    /**
     * Clears all lines from the code block.
     */
    void clear();

    /**
     * Returns whether the code block is empty.
     */
    bool isEmpty() const;

    /**
     * Sets the @param indent of the code block.
     */
    void setIndent( int indent );

    /**
     * Indents the code block by one level.
     */
    void indent();

    /**
     * Unindents the code block by one level.
     */
    void unindent();

    /**
     * Returns the textual presentation of the code block.
     */
    QString text() const;

    /**
     * Adds the given @param line to the code block.
     */
    void addLine( const QString &line );

    /**
     * Adds the given @param line to the code block.
     */
    void addLine( const char line );

    /**
     * Adds the given @param block to the code block.
     * The current indent will be prepended before every line of the block.
     */
    void addBlock( const Code &block );

    /**
     * Adds the given @param block to the code block.
     * The current indent will be prepended before every line of the block.
     */
    void addBlock( const QString &block );

    /**
     * Adds the given @param block with the given indent
     * to the code block.
     */
    void addBlock( const QString &block, int indent );

    /**
     * Adds the given @param text to the code block and wrapps
     * it if it's too long.
     */
    void addWrappedText( const QString &text );

    /**
     * Adds the given @param text to the code block and wrapps
     * it at word boundaries if it's too long.
     */
    void addFormattedText( const QString &text );

    /**
     * Adds a new line to the code block.
     */
    void newLine();

    /**
     * Adds the given @param line to the code block
     * and appends a '\r\n' automatically.
     */
    Code &operator+=( const QString &line );

    /**
     * Adds the given @param line to the code block
     * and appends a '\r\n' automatically.
     */
    Code &operator+=( const QByteArray& line );

    /**
     * Adds the given @param line to the code block
     * and appends a '\r\n' automatically.
     */
    Code &operator+=( const char *line );

    /**
     * Adds the given @param line to the code block
     * and appends a '\r\n' automatically.
     */
    Code &operator+=( const char line );

    /**
     * Adds the given @param block to the code block.
     * @param block is supposed to be fully indented already
     * (otherwise, use addBlock)
     */
    Code &operator+=( const Code &block );

    /**
     * Returns a string filled up with spaces, depending on
     * the level @param count and the indent value.
     */
    static QString spaces( int count );

    /**
     * Sets the default indentation used by indent().
     */
    static void setDefaultIndentation( int indent );

    /**
     * Returns the default indentation used by indent().
     */
    static int defaultIndentation();

  private:
    class Private;
    Private *d;
};

}

#endif
