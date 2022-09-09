/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_PORTTYPE_H
#define KWSDL_PORTTYPE_H

#include <QDomElement>
#include <QList>

#include <wsdl/element.h>
#include <wsdl/operation.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT PortType : public Element
{
public:
    typedef QList<PortType> List;

    PortType();
    PortType(const QString &nameSpace);
    ~PortType();

    void setName(const QString &name);
    QString name() const;

    void setOperations(const Operation::List &operations);
    Operation::List operations() const;

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    Operation::List mOperations;
};

}

#endif // KWSDL_PORTTYPE_H
