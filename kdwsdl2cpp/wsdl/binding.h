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

#ifndef KWSDL_BINDING_H
#define KWSDL_BINDING_H

#include <QDomElement>
#include <QList>

#include <common/qname.h>
#include <wsdl/bindingoperation.h>
#include <wsdl/element.h>
#include <wsdl/soapbinding.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL
{

class KWSDL_EXPORT Binding : public Element
{
public:
    typedef QList<Binding> List;

    enum Type {
        SOAPBinding,
        HTTPBinding,
        MIMEBinding,
        UnknownBinding
    };

    Binding();
    Binding(const QString &nameSpace);
    ~Binding();

    void setName(const QString &name);
    QString name() const;

    void setPortTypeName(const QName &portTypeName);
    QName portTypeName() const;

    void setType(Type type);
    Type type() const;

    enum Version {
        SOAP_1_1,
        SOAP_1_2
    };
    void setVersion(Version v);
    Version version() const;

    void setOperations(const BindingOperation::List &operations);
    BindingOperation::List operations() const;

    //void setSoapBinding( const SoapBinding &soapBinding );
    SoapBinding soapBinding() const;

    const AbstractBinding *binding() const;

    void loadXML(ParserContext *context, const QDomElement &element);
    void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    QString mName;
    QName mPortTypeName;
    BindingOperation::List mOperations;

    Type mType;
    SoapBinding mSoapBinding;
    Version mVersion;
};

}

#endif // KWSDL_BINDING_H

