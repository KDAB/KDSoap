/*
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

#ifndef KWSDL_CONVERTER_H
#define KWSDL_CONVERTER_H

#include <common/nsmanager.h>
#include <libkode/class.h>
#include <schema/parser.h>
#include <wsdl/wsdl.h>
#include <QSet>

#include "namemapper.h"
#include "typemap.h"

#ifdef NDEBUG
#define COMMENT QString()
#else
#define COMMENT QLatin1String("// ") + Converter::shortenFilename(QLatin1String(__FILE__)) + QLatin1String(":") + QString::number(__LINE__)
#endif

namespace KWSDL
{

class Converter
{
public:
    Converter();

    void setWSDL(const WSDL &wsdl);

    bool convert();

    KODE::Class::List classes() const;

    static QString shortenFilename(const QString &path);

private:

    void cleanupUnusedTypes();
    void convertTypes();

    void convertComplexType(const XSD::ComplexType *);
    void createComplexTypeSerializer(KODE::Class &, const XSD::ComplexType *);

    void convertSimpleType(const XSD::SimpleType *, const XSD::SimpleType::List &simpleTypeList);
    void createSimpleTypeSerializer(KODE::Class &, const XSD::SimpleType *, const XSD::SimpleType::List &simpleTypeList);

    // Client Stub
    bool convertClientService();
    bool convertClientCall(const Operation &, const Binding &, KODE::Class &);
    void convertClientInputMessage(const Operation &, const Binding &, KODE::Class &);
    void convertClientOutputMessage(const Operation &, const Binding &, KODE::Class &);
    void clientAddOneArgument(KODE::Function &callFunc, const Part &part, KODE::Class &newClass);
    void clientAddArguments(KODE::Function &callFunc, const Message &message, KODE::Class &newClass, const Operation &operation, const Binding &binding);
    bool clientAddAction(KODE::Code &code, const Binding &binding, const QString &operationName);
    void clientGenerateMessage(KODE::Code &code, const Binding &binding, const Message &message, const Operation &operation, bool varsAreMembers = false);
    void addMessageArgument(KODE::Code &code, const SoapBinding::Style &bindingStyle, const Part &part, const QString &localVariableName, const QByteArray &messageName, bool varIsMember = false);
    void createHeader(const SoapBinding::Header &header, KODE::Class &newClass);
    void addJobResultMember(KODE::Class &jobClass, const Part &part, const QString &varName, const QStringList &inputGetters);
    KODE::Code serializePart(const Part &part, const QString &localVariableName, const QString &varName, bool append);
    KODE::Code demarshalVarHelper(const QName &type, const QName &elementType, const QString &variableName, const QString &qtTypeName, const QString &soapValueVarName, bool optional) const;
    KODE::Code demarshalVar(const QName &type, const QName &elementType, const QString &variableName, const QString &typeName, const QString &soapValueVarName, bool optional, bool usePointer) const;
    KODE::Code demarshalArrayVar(const QName &type, const QString &variableName, const QString &qtTypeName) const;
    void addVariableInitializer(KODE::MemberVariable &variable) const;
    QString generateMemberVariable(const QString &rawName, const QString &typeName, const QString &inputTypeName, KODE::Class &newClass, XSD::Attribute::AttributeUse, bool usePointer, bool polymorphic);
    QString listTypeFor(const QString &itemTypeName, KODE::Class &newClass);
    KODE::Code deserializeRetVal(const KWSDL::Part &part, const QString &replyMsgName, const QString &qtRetType, const QString &varName) const;
    QName elementNameForPart(const Part &part, bool *qualified, bool *nillable) const;
    bool isQualifiedPart(const Part &part) const;

    // Server Stub
    void convertServerService();
    void generateServerMethod(KODE::Code &code, const Binding &binding, const Operation &operation,
                              KODE::Class &newClass, bool first);
    void generateDelayedReponseMethod(const QString &methodName, const QString &retInputType,
                                      const Part &retPart, KODE::Class &newClass, const Binding &binding, const Message &outputMessage);

    SoapBinding::Style soapStyle(const Binding &binding) const;

    WSDL mWSDL;

    KODE::Class::List mClasses;
    KODE::Class mQObject;
    KODE::Class mKDSoapServerObjectInterface;

    NameMapper mNameMapper;
    TypeMap mTypeMap;
    NSManager mNSManager;
    QSet<QString> mHeaderMethods;
};

}

QString upperlize(const QString &);
QString lowerlize(const QString &);
QString namespaceString(const QString &ns);

static QName XmlAnyType(QLatin1String("http://www.w3.org/2001/XMLSchema"), QLatin1String("any"));

#endif
