/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef KWSDL_ELEMENT_H
#define KWSDL_ELEMENT_H

#include <QString>

#include <kode_export.h>

namespace KWSDL
{

class KWSDL_EXPORT Element
{
public:
    Element();
    Element(const QString &nameSpace);
    ~Element();

    void setNameSpace(const QString &nameSpace);
    QString nameSpace() const;

    void setDocumentation(const QString &documentation);
    QString documentation() const;

private:
    QString mNameSpace;
    QString mDocumentation;
};

}

#endif // KWSDL_ELEMENT_H

