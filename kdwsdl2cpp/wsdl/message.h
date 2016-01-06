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

#ifndef KWSDL_MESSAGE_H
#define KWSDL_MESSAGE_H

#include <QDomElement>
#include <QList>

#include <wsdl/element.h>
#include <wsdl/part.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL
{

class KWSDL_EXPORT Message : public Element
{
public:
    typedef QList<Message> List;

    Message();
    Message(const QString &nameSpace);
    ~Message();

    void setName(const QString &name);
    QString name() const;

    void setParts(const Part::List &parts);
    Part::List parts() const;
    Part partByName(const QString &name) const;

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    Part::List mParts;
};

}

#endif // KWSDL_MESSAGE_H

