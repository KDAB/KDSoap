/*
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2010 David Faure <david.faure@kdab.com>

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

#include "namemapper.h"

using namespace KWSDL;

NameMapper::NameMapper()
{
    // From http://www.cppreference.com/wiki/keywords/start
    mKeyWords << "and" << "and_eq" << "asm" << "auto" << "bitand" << "bitor"
              << "bool" << "break" << "case" << "catch" << "char" << "class"
              << "compl" << "const" << "const_cast" << "continue" << "default"
              << "delete" << "do" << "double" << "dynamic_cast" << "else"
              << "enum" << "explicit" << "export" << "extern" << "false" << "FALSE"
              << "float" << "for" << "friend" << "goto" << "if" << "inline"
              << "int" << "long" << "mutable" << "namespace" << "new" << "not"
              << "not_eq" << "operator" << "or" << "or_eq" << "private" << "protected"
              << "public" << "register" << "reinterpret_cast" << "return" << "short"
              << "signed" << "sizeof" << "static" << "static_cast" << "struct"
              << "switch" << "template" << "this" << "throw" << "true" << "TRUE" << "try"
              << "typedef" << "typeid" << "typename" << "union" << "unsigned"
              << "using" << "virtual" << "void" << "volatile" << "wchar_t"
              << "while" << "xor" << "xor_eq";
    // Also some Qt keywords:
    mKeyWords << "emit" << "signals" << "slots" << "foreach";
}

QString NameMapper::escape(const QString &name) const
{
    if (mKeyWords.contains(name)) {
        return name + '_';
    } else {
        return name;
    }
}

QString NameMapper::unescape(const QString &name) const
{
    if (name.startsWith("_")) {
        return name.mid(1);
    } else {
        return name;
    }
}
