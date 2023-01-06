/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
 Author: David Faure <david.faure@kdab.com>

 SPDX-License-Identifier: MIT
*/

#include "namemapper.h"

using namespace KWSDL;

NameMapper::NameMapper()
{
    // From https://en.cppreference.com/w/cpp/keyword
    mKeyWords << "and"
              << "and_eq"
              << "asm"
              << "auto"
              << "bitand"
              << "bitor"
              << "bool"
              << "break"
              << "case"
              << "catch"
              << "char"
              << "class"
              << "compl"
              << "const"
              << "const_cast"
              << "continue"
              << "default"
              << "delete"
              << "do"
              << "double"
              << "dynamic_cast"
              << "else"
              << "enum"
              << "explicit"
              << "export"
              << "extern"
              << "false"
              << "FALSE"
              << "float"
              << "for"
              << "friend"
              << "goto"
              << "if"
              << "inline"
              << "int"
              << "long"
              << "mutable"
              << "namespace"
              << "new"
              << "not"
              << "not_eq"
              << "operator"
              << "or"
              << "or_eq"
              << "private"
              << "protected"
              << "public"
              << "register"
              << "reinterpret_cast"
              << "return"
              << "short"
              << "signed"
              << "sizeof"
              << "static"
              << "static_cast"
              << "struct"
              << "switch"
              << "template"
              << "this"
              << "throw"
              << "true"
              << "TRUE"
              << "try"
              << "typedef"
              << "typeid"
              << "typename"
              << "union"
              << "unsigned"
              << "using"
              << "virtual"
              << "void"
              << "volatile"
              << "wchar_t"
              << "while"
              << "xor"
              << "xor_eq";
    // Also some Qt keywords:
    mKeyWords << "emit"
              << "signals"
              << "slots"
              << "foreach";
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
