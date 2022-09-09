/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_FAULT_H
#define KWSDL_FAULT_H

#include <QDomElement>
#include <QList>

#include <common/qname.h>
#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Fault : public Element
{
public:
    typedef QList<Fault> List;

    Fault();
    Fault(const QString &nameSpace);
    ~Fault();

    void setName(const QString &name);
    QString name() const;

    void setMessage(const QName &message);
    QName message() const;

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    QName mMessage;
};

}

#endif // KWSDL_FAULT_H
