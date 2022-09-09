/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_TYPE_H
#define KWSDL_TYPE_H

#include <qdom.h>

#include <schema/types.h>
#include <wsdl/element.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Type : public Element
{
public:
    Type();
    Type(const QString &nameSpace);
    ~Type();

    void setTypes(const XSD::Types &types);
    XSD::Types types() const;

    bool loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    XSD::Types mTypes;
};

}

#endif // KWSDL_TYPE_H
