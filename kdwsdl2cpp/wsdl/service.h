/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_SERVICE_H
#define KWSDL_SERVICE_H

#include <QDomElement>
#include <QList>

#include <wsdl/binding.h>
#include <wsdl/element.h>
#include <wsdl/port.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Service : public Element
{
public:
    typedef QList<Service> List;

    Service();
    Service(const QString &nameSpace);
    ~Service();

    void setName(const QString &name);
    QString name() const;

    void setPorts(const Port::List &ports);
    Port::List ports() const;

    void loadXML(ParserContext *context, Binding::List *bindings, const QDomElement &element);
    void saveXML(ParserContext *context, const Binding::List *bindings, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    Port::List mPorts;
};

}

#endif // KWSDL_SERVICE_H
