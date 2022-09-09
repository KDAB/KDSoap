/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_DEFINITIONS_H
#define KWSDL_DEFINITIONS_H

#include <qdom.h>
#include <qurl.h>

#include <wsdl/binding.h>
#include <wsdl/import.h>
#include <wsdl/message.h>
#include <wsdl/porttype.h>
#include <wsdl/service.h>
#include <wsdl/type.h>

#include <kode_export.h>

class ParserContext;

namespace KWSDL {

class KWSDL_EXPORT Definitions
{
public:
    Definitions();
    ~Definitions();

    void setWantedService(const QString &name);

    void setUseLocalFilesOnly(bool localFilesOnly);

    void setName(const QString &name);
    QString name() const;

    void setTargetNamespace(const QString &targetNamespace);
    QString targetNamespace() const;

    // void setBindings( const Binding::List &bindings );
    Binding::List bindings() const;

    // void setImports( const Import::List &imports );
    // Import::List imports() const;

    void setMessages(const Message::List &messages);
    Message::List messages() const;

    void setPortTypes(const PortType::List &portTypes);
    PortType::List portTypes() const;

    // void setService( const Service &service );
    Service::List services() const;

    void setType(const Type &type);
    Type type() const;

    bool loadXML(ParserContext *context, const QDomElement &element);
    // void saveXML( ParserContext *context, QDomDocument &document ) const;

    void fixUpDefinitions(/*ParserContext *context, const QDomElement &element*/);

private:
    void importDefinition(ParserContext *context, const QString &location);

    Binding::List mBindings;
    // Import::List mImports;
    Message::List mMessages;
    PortType::List mPortTypes;
    Service::List mServices;
    Type mType;

    QString mTargetNamespace;
    QString mName;
    QString mWantedService;

    bool mUseLocalFilesOnly = false;
};


}

#endif // KWSDL_DEFINITIONS_H
