/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_BINDINGOPERATION_H
#define KWSDL_BINDINGOPERATION_H

#include <QDomElement>
#include <QList>

#include <common/qname.h>
#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class AbstractBinding;

class KWSDL_EXPORT BindingOperation : public Element
{
public:
    typedef QList<BindingOperation> List;

    BindingOperation();
    BindingOperation(const QString &nameSpace);
    ~BindingOperation();

    void setName(const QString &name);
    QString name() const;

    void loadXML(AbstractBinding *binding, ParserContext *context, const QDomElement &element);
    void saveXML(const AbstractBinding *binding, ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
};

}

#endif
