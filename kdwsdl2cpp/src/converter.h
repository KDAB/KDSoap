/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#ifndef KWSDL_CONVERTER_H
#define KWSDL_CONVERTER_H

#include <QSet>
#include <code_generation/class.h>
#include <common/nsmanager.h>
#include <schema/parser.h>
#include <wsdl/wsdl.h>

#include "namemapper.h"
#include "typemap.h"

#ifdef NDEBUG
#define COMMENT QString()
#else
#define COMMENT QLatin1String("// ") + Converter::shortenFilename(QLatin1String(__FILE__)) + QLatin1String(":") + QString::number(__LINE__)
#endif

namespace KWSDL {

class Converter
{
public:
    struct DefaultAttributeValue
    {
        DefaultAttributeValue(bool isBuiltin = false, const QString &value = QString())
            : mIsBuiltin(isBuiltin)
            , mValue(value)
        {
        }

        bool mIsBuiltin;
        QString mValue;

        bool isNull() const
        {
            return mValue.isNull();
        }
    };

    Converter();

    void setWSDL(const WSDL &wsdl);

    bool convert();

    KODE::Class::List classes() const;

    static QString shortenFilename(const QString &path);
    static QName XmlAnyType()
    {
        return QName(QLatin1String("http://www.w3.org/2001/XMLSchema"), QLatin1String("any"));
    }

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
    void clientAddArguments(KODE::Function &callFunc, const Message &message, KODE::Class &newClass, const Operation &operation,
                            const Binding &binding);
    bool clientAddAction(KODE::Code &code, const Binding &binding, const QString &operationName);
    void clientGenerateMessage(KODE::Code &code, const Binding &binding, const Message &message, const Operation &operation,
                               bool varsAreMembers = false);
    void addMessageArgument(KODE::Code &code, SoapBinding::Style bindingStyle, const Part &part, const QString &localVariableName,
                            const QByteArray &messageName, bool varIsMember = false);
    void createHeader(const SoapBinding::Header &header, KODE::Class &newClass);
    void addJobResultMember(KODE::Class &jobClass, const Part &part, const QString &varName, const QStringList &inputGetters);
    KODE::Code serializePart(const Part &part, const QString &localVariableName, const QString &nilVariableName, const QString &varName, bool append);

    void addVariableInitializer(KODE::MemberVariable &variable) const;

    // Implements default values processing ONLY for attributes
    QString generateMemberVariable(const QString &rawName, const QString &typeName, const QString &inputTypeName, KODE::Class &newClass,
                                   XSD::Attribute::AttributeUse, bool usePointer, bool polymorphic,
                                   const DefaultAttributeValue &defaultValue = DefaultAttributeValue());

    QString listTypeFor(const QString &itemTypeName, KODE::Class &newClass);
    KODE::Code deserializeRetVal(const KWSDL::Part &part, const QString &replyMsgName, const QString &qtRetType, const QString &varName) const;
    QName elementNameForPart(const Part &part, bool *qualified, bool *nillable) const;
    bool isQualifiedPart(const Part &part) const;

    // Server Stub
    void convertServerService();
    void generateServerMethod(KODE::Code &code, const Binding &binding, const Operation &operation, KODE::Class &newClass, bool first);
    void generateDelayedReponseMethod(const QString &methodName, const QString &retInputType, const Part &retPart, KODE::Class &newClass,
                                      const Binding &binding, const Message &outputMessage);

    SoapBinding::Style soapStyle(const Binding &binding) const;

    WSDL mWSDL;

    KODE::Class::List mClasses;
    KODE::Class::List mServerClasses;
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


#endif
