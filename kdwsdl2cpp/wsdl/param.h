/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_PARAM_H
#define KWSDL_PARAM_H

#include <QDomElement>
#include <QList>

#include <common/qname.h>
#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Param : public Element
{
public:
    typedef QList<Param> List;

    Param();
    Param(const QString &nameSpace);
    ~Param();

    void setName(const QString &name);
    QString name() const;

    void setMessage(const QName &message);
    QName message() const;

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    QName mMessage;
};

}

#endif // KWSDL_PARAM_H
