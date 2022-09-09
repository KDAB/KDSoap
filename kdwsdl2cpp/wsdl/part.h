/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_PART_H
#define KWSDL_PART_H

#include <QDomElement>
#include <QList>

#include <common/qname.h>
#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Part : public Element
{
public:
    typedef QList<Part> List;

    Part();
    Part(const QString &nameSpace);
    ~Part();

    void setName(const QString &name);
    QString name() const;

    void setType(const QName &type);
    QName type() const;

    void setElement(const QName &element);
    QName element() const;

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    QName mType;
    QName mElement;
};

}

#endif // KWSDL_PART_H
