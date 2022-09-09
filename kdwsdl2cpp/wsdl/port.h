/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_PORT_H
#define KWSDL_PORT_H

#include <QDomElement>
#include <QList>

#include <common/qname.h>
#include <wsdl/binding.h>
#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Port : public Element
{
public:
    typedef QList<Port> List;

    Port();
    Port(const QString &nameSpace);
    ~Port();

    void setName(const QString &name);
    QString name() const;

    void setBindingName(const QName &bindingName);
    QName bindingName() const;

    void loadXML(ParserContext *context, Binding::List *bindings, const QDomElement &element);
    void saveXML(ParserContext *context, const Binding::List *bindings, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    QName mBindingName;
};

}

#endif // KWSDL_PORT_H
