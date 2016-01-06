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

#ifndef KWSDL_SOAPBINDING_H
#define KWSDL_SOAPBINDING_H

#include <QDomElement>
#include <QMap>
#include <QUrl>

#include <common/qname.h>
#include <wsdl/abstractbinding.h>

#include <kode_export.h>

namespace KWSDL
{

class KWSDL_EXPORT SoapBinding : public AbstractBinding
{
public:
    enum Style {
        RPCStyle,
        DocumentStyle
    };

    enum Use {
        LiteralUse,
        EncodedUse
    };

    enum Transport {
        HTTPTransport
    };

    class KWSDL_EXPORT Binding
    {
    public:
        Binding();
        ~Binding();

        void setTransport(Transport transport);
        Transport transport() const;

        void setStyle(Style style);
        Style style() const;

        void loadXML(ParserContext *context, const QDomElement &element);
        void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

    private:
        Transport mTransport;
        Style mStyle;
    };

    class KWSDL_EXPORT Body
    {
    public:
        Body();
        ~Body();

#if 0
        void setEncodingStyle(const QString &encodingStyle);
        QString encodingStyle() const;
#endif

        //void setPart( const QString &part );
        QString part() const;

        //void setUse( Use use );
        Use use() const;

        //void setNameSpace( const QString &nameSpace );
        QString nameSpace() const;

        void loadXML(ParserContext *context, const QDomElement &element);
        void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

    private:
#if 0
        QString mEncodingStyle;
#endif
        QString mPart;
        Use mUse;
        QString mNameSpace;
    };

    class KWSDL_EXPORT Fault
    {
    public:
        Fault();
        ~Fault();

#if 0
        void setEncodingStyle(const QString &encodingStyle);
        QString encodingStyle() const;
#endif

        void setUse(Use use);
        Use use() const;

        void setNameSpace(const QString &nameSpace);
        QString nameSpace() const;

        void loadXML(ParserContext *context, const QDomElement &element);
        void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

    private:
#if 0
        QString mEncodingStyle;
#endif
        Use mUse;
        QString mNameSpace;
    };

    class KWSDL_EXPORT HeaderFault
    {
    public:
        HeaderFault();
        ~HeaderFault();

        void loadXML(ParserContext *context, const QDomElement &element);
        void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

        void setMessage(const QName &message);
        QName message() const;

#if 0
        void setEncodingStyle(const QString &encodingStyle);
        QString encodingStyle() const;
#endif

        void setPart(const QString &part);
        QString part() const;

        void setUse(Use use);
        Use use() const;

        void setNameSpace(const QString &nameSpace);
        QString nameSpace() const;

    private:
        QName mMessage;
        QString mPart;
        Use mUse;
#if 0
        QString mEncodingStyle;
#endif
        QString mNameSpace;
    };

    class Headers;

    class KWSDL_EXPORT Header
    {
    public:
        Header();
        ~Header();

        void loadXML(ParserContext *context, const QDomElement &element);
        void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

        void setHeaderFault(const HeaderFault &headerFault);
        HeaderFault headerFault() const;

        void setMessage(const QName &message);
        QName message() const;

        void setPart(const QString &part);
        QString part() const;

        void setUse(const Use &use);
        Use use() const;

#if 0
        void setEncodingStyle(const QString encodingStyle);
        QString encodingStyle() const;
#endif
        void setNameSpace(const QString &nameSpace);
        QString nameSpace() const;

    private:
        friend class Headers;
        HeaderFault mHeaderFault;
        QName mMessage;
        QString mPart;
        Use mUse;
#if 0
        QString mEncodingStyle;
#endif
        QString mNameSpace;
    };

    class Headers : public QList<Header>
    {
    public:
        bool contains(const Header &header) const;
    };

    /**
     * <operation> as defined in a soap <binding>.
     * Contains some flags for input and output (e.g. <output><soap:body use="literal"/></output>
     * but also the full definition of <soap:headers> for <input>.
     */
    class KWSDL_EXPORT Operation
    {
    public:
        typedef QList<Operation> List;
        typedef QMap<QString, Operation> Map;

        Operation();
        Operation(const QString &name);
        ~Operation();

        void loadXML(ParserContext *context, const QDomElement &element);
        void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

        void setName(const QString &name);
        QString name() const;

        void setAction(const QString &action);
        QString action() const;

        void setStyle(Style style);
        Style style() const;

        void setInput(const Body &input);
        Body input() const;

        void setOutput(const Body &output);
        Body output() const;

        void addInputHeader(const Header &inputHeader);
        Headers inputHeaders() const;

        void addOutputHeader(const Header &outputHeader);
        Headers outputHeaders() const;

        void setFault(const Fault &fault);
        Fault fault() const;

    private:
        QString mName;
        QString mSoapAction;
        Style mStyle;
        Body mInputBody;
        Headers mInputHeaders;
        Body mOutputBody;
        Headers mOutputHeaders;
        Fault mFault;
    };

    class KWSDL_EXPORT Address
    {
    public:
        Address();
        ~Address();

        void setLocation(const QUrl &location);
        QUrl location() const;

        void loadXML(ParserContext *context, const QDomElement &element);
        void saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

    private:
        QUrl mLocation;
    };

    SoapBinding();
    ~SoapBinding();

    void setAddress(const Address &address);
    Address address() const;

    void setOperations(const Operation::Map &operations);
    Operation::Map operations() const;

    void setBinding(const Binding &binding);
    Binding binding() const;

    void parseBinding(ParserContext *context, const QDomElement &parent);
    void parseOperation(ParserContext *context, const QString &name, const QDomElement &parent);
    void parseOperationInput(ParserContext *context, const QString &name, const QDomElement &parent);
    void parseOperationOutput(ParserContext *context, const QString &name, const QDomElement &parent);
    void parseOperationFault(ParserContext *context, const QString &name, const QDomElement &parent);
    void parsePort(ParserContext *context, const QDomElement &parent);

    void synthesizeBinding(ParserContext *context, QDomDocument &document, QDomElement &parent) const;
    void synthesizeOperation(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const;
    void synthesizeOperationInput(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const;
    void synthesizeOperationOutput(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const;
    void synthesizeOperationFault(ParserContext *context, const QString &name, QDomDocument &document, QDomElement &parent) const;
    void synthesizePort(ParserContext *context, QDomDocument &document, QDomElement &parent) const;

private:
    Binding mBinding;
    Operation::Map mOperations;
    Address mAddress;
};

}

#endif // KWSDL_SOAPBINDING_H

