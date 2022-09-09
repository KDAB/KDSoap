/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_MESSAGE_H
#define KWSDL_MESSAGE_H

#include <QDomElement>
#include <QList>

#include <wsdl/element.h>
#include <wsdl/part.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

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
