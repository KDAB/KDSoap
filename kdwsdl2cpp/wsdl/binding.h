/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
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

namespace KWSDL {

class KWSDL_EXPORT Binding : public Element
{
public:
    typedef QList<Binding> List;

    enum Type
    {
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

    enum Version
    {
        SOAP_1_1,
        SOAP_1_2
    };
    void setVersion(Version v);
    Version version() const;

    void setOperations(const BindingOperation::List &operations);
    BindingOperation::List operations() const;

    // void setSoapBinding( const SoapBinding &soapBinding );
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
