/*
 SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>

 SPDX-License-Identifier: MIT
*/

#include "converter.h"
#include "elementargumentserializer.h"
#include "settings.h"
#include <code_generation/style.h>

#include <QDebug>

using namespace KWSDL;

// Return true if the generated code should remember whether this elem was set by the user.
static bool isElementOptional(const XSD::Element &elem)
{
    return elem.minOccurs() == 0 || elem.compositor().type() == XSD::Compositor::Choice || elem.compositor().minOccurs() == 0;
}

static bool isElementPolymorphic(const XSD::Element &elem, const KWSDL::TypeMap &typeMap, bool isList)
{
    const QString typeName = typeMap.localType(elem.type());
    bool polymorphic = typeMap.isPolymorphic(elem.type()) && !isList;
    if (polymorphic && (typeName == "QString" || typeName == "bool")) {
        qWarning() << "Shouldn't happen: polymorphic" << typeName << ". Comes from element" << elem.name() << "type" << elem.type();
        Q_ASSERT(0);
    }
    return polymorphic;
}

static bool usePointerForElement(const XSD::Element &elem, const KODE::Class &newClass, const KWSDL::TypeMap &typeMap, bool isList)
{
    const QString typeName = typeMap.localType(elem.type());
    const bool polymorphic = isElementPolymorphic(elem, typeMap, isList);
    if (!isList && isElementOptional(elem) && newClass.qualifiedName() == typeName) {
        if (qEnvironmentVariableIsSet("KDSOAP_TYPE_DEBUG")) {
            qDebug() << "Using pointer for optional type" << typeName << "used inside itself";
        }
        // Optional "Foo" in class "Foo" - can't just have a value and bool, we need a pointer, to avoid infinite recursion
        // use == XSD::Attribute::Required;
        return true;
    }
    return polymorphic;
}

static void generateDefaultAttributeValueCode(KODE::Code &result, const QString &typeName, const Converter::DefaultAttributeValue &defaultValue)
{
    result += "{";
    result.indent();

    if (!defaultValue.mIsBuiltin) {
        result += typeName + " defaultValue;";
        result += "defaultValue.deserialize(KDSoapValue(QString(), \"" + defaultValue.mValue + "\"));";
        result += "return defaultValue;";
    } else {
        result += "const QString defaultValueFromWsdl(\"" + defaultValue.mValue + "\");";
        if (typeName == "KDDateTime") {
            result += "KDDateTime defaultDateTime = KDDateTime::fromDateString(defaultValueFromWsdl);";
            result += "if (defaultDateTime.toDateString() != defaultValueFromWsdl) {";
            result.indent();
            result += "qDebug(\"Can't convert to " + typeName + " from " + defaultValue.mValue + "\");";
            result += "Q_ASSERT(!\"Can't convert to " + typeName + " from " + defaultValue.mValue + "\");";
            result.unindent();
            result += "}";
            result += "return defaultDateTime;";
        } else {
            result += "const QVariant tmp(defaultValueFromWsdl);";
            result += "if(!tmp.canConvert<" + typeName + ">())";
            result += "{";
            result.indent();
            result += "qDebug(\"Can't convert to " + typeName + " from " + defaultValue.mValue + "\");";
            result += "Q_ASSERT(!\"Can't convert to " + typeName + " from " + defaultValue.mValue + "\");";
            result.unindent();
            result += "}";
            result += "return tmp.value<" + typeName + ">();";
        }
    }

    result.unindent();
    result += "}";
}

void Converter::convertComplexType(const XSD::ComplexType *type)
{
    // An empty type is still useful, in document mode: it serializes the element name
    // if ( type->isEmpty() )
    //    return;

    const QString className(mTypeMap.localType(type->qualifiedName()));
    KODE::Class newClass;
    newClass.setNamespaceAndName(className);
    if (!Settings::self()->exportDeclaration().isEmpty()) {
        newClass.setExportDeclaration(Settings::self()->exportDeclaration());
    }

    newClass.setUseSharedData(true, QLatin1String("d_ptr") /*avoid clash with possible d() method */);

    if (qEnvironmentVariableIsSet("KDSOAP_TYPE_DEBUG")) {
        qDebug() << "Generating complex type" << className;
    }

    // subclass handling
    if (!type->baseTypeName().isEmpty()) { // this class extends something
        /**
         * A class can't subclass basic type (e.g. int, unsigned char), so we
         * add setValue() and value() methods to access the base type.
         *
         * In fact, let's do the same with string
         */
        if (type->baseTypeName().localName() == QLatin1String("Array")) {
            // this is handled in the attribute section
        } else {
            const QName baseName = type->baseTypeName();
            const QString typeName = mTypeMap.localType(baseName);
            const QString inputTypeName = mTypeMap.localInputType(baseName, QName());

            // include header
            newClass.addIncludes(QStringList(), mTypeMap.forwardDeclarations(baseName));
            newClass.addHeaderIncludes(mTypeMap.headerIncludes(baseName));

            if (mTypeMap.isComplexType(baseName)) {
                newClass.addBaseClass(typeName);
            } else {
                const QString variableName =
                    generateMemberVariable("value", typeName, inputTypeName, newClass, XSD::Attribute::Required, false, false);

                // convenience constructor
                KODE::Function conctor(upperlize(newClass.name()));
                conctor.addArgument(inputTypeName + QLatin1String(" value"));
                conctor.setBody(variableName + QLatin1String(" = value;"));

                // type operator
                KODE::Function op(QLatin1String("operator ") + typeName);
                op.setBody(QLatin1String("return ") + variableName + QLatin1Char(';'));
                op.setConst(true);

                newClass.addFunction(conctor);
                newClass.addFunction(op);
            }
        }
    }

    if (!type->documentation().isEmpty()) {
        newClass.setDocs(type->documentation().simplified());
    }

    QVector<QString> seenElements;

    // elements in the complex type
    const XSD::Element::List elements = type->elements();
    for (const XSD::Element &elemIt : elements) {

        if (elemIt.type().isEmpty()) {
            qDebug() << "ERROR: Element from" << *type << "with no type:" << elemIt << "(skipping)";
            Q_ASSERT(false);
            continue;
        }

        // When having <choice>
        //                <sequence>A,B(opt)</sequence>
        //                B
        //                <sequence>C,B(opt)</sequence>
        //             </choice>
        // we don't want to emit setB() three times, that's not valid C++
        // (testcase in wsdl_document.wsdl TestRepeatedChildren)
        if (seenElements.contains(elemIt.name())) {
            continue;
        }
        seenElements.append(elemIt.name());

        QString typeName = mTypeMap.localType(elemIt.type());
        Q_ASSERT(!typeName.isEmpty());

        if (typeName != QLatin1String("void")) { // void means empty element, probably just here for later extensions (testcase: SetPasswordResult in
                                                 // salesforce)
            QString inputTypeName = mTypeMap.localInputType(elemIt.type(), QName());

            bool isList = false;
            if (elemIt.maxOccurs() > 1 || elemIt.compositor().maxOccurs() > 1) {
                QString itemType = mTypeMap.isPolymorphic(elemIt.type()) ? ElementArgumentSerializer::pointerStorageType(typeName) : typeName;
                typeName = listTypeFor(itemType, newClass);
                inputTypeName = QLatin1String("const ") + typeName + QLatin1String("&");
                isList = true;
            }
            if (type->isArray()) {
                QString arrayTypeName = mTypeMap.localType(type->arrayType());
                Q_ASSERT(!arrayTypeName.isEmpty());
                if (mTypeMap.isPolymorphic(type->arrayType())) {
                    arrayTypeName = ElementArgumentSerializer::pointerStorageType(arrayTypeName);
                }
                // qDebug() << "array of" << attribute.arrayType() << "->" << arrayTypeName;
                typeName = listTypeFor(arrayTypeName, newClass);
                if (!mTypeMap.isBasicType(type->arrayType())) {
                    newClass.addInclude(QString(), arrayTypeName); // add forward declaration
                }
                newClass.addHeaderIncludes(QStringList() << QLatin1String("QtCore/QList"));
                inputTypeName = QLatin1String("const ") + typeName + QLatin1Char('&');
                isList = true;
            }

            XSD::Attribute::AttributeUse use = isElementOptional(elemIt) ? XSD::Attribute::Optional : XSD::Attribute::Required;
            const bool polymorphic = isElementPolymorphic(elemIt, mTypeMap, isList);
            const bool usePointer = usePointerForElement(elemIt, newClass, mTypeMap, isList);
            generateMemberVariable(KODE::Style::makeIdentifier(elemIt.name()), typeName, inputTypeName, newClass, use, usePointer, polymorphic);
        }

        // include header
        newClass.addIncludes(QStringList(), mTypeMap.forwardDeclarations(elemIt.type()));
        newClass.addHeaderIncludes(mTypeMap.headerIncludes(elemIt.type()));
        if (elemIt.maxOccurs() > 1 || elemIt.compositor().maxOccurs() > 1) {
            newClass.addHeaderIncludes(QStringList() << QLatin1String("QtCore/QList"));
        }
    }

    // attributes in the complex type
    const XSD::Attribute::List attributes = type->attributes();
    for (const XSD::Attribute &attribute : attributes) {
        QString typeName, inputTypeName;

        typeName = mTypeMap.localType(attribute.type());
        if (typeName.isEmpty()) {
            qWarning() << "ERROR: attribute with unknown type:" << attribute.name() << attribute.type() << "in" << typeName;
            continue;
        }
        inputTypeName = mTypeMap.localInputType(attribute.type(), QName());
        // qDebug() << "Attribute" << attribute.name();

        bool isBuiltin = mTypeMap.isBuiltinType(attribute.type());
        QString value = attribute.defaultValue();
        DefaultAttributeValue defaultValue(isBuiltin, value);
        generateMemberVariable(KODE::Style::makeIdentifier(attribute.name()), typeName, inputTypeName, newClass, attribute.attributeUse(), false,
                               false, defaultValue);

        // include header
        newClass.addIncludes(QStringList(), mTypeMap.forwardDeclarations(attribute.type()));
        newClass.addHeaderIncludes(mTypeMap.headerIncludes(attribute.type()));
    }

    createComplexTypeSerializer(newClass, type);

    const QString newClassName = newClass.name();

    KODE::Function ctor(/*upperlize*/ (newClassName));
    newClass.addFunction(ctor);

    KODE::Function dtor(QLatin1Char('~') + /*upperlize*/ (newClassName));
    if (!type->derivedTypes().isEmpty()) {
        dtor.setVirtualMode(KODE::Function::Virtual);
    }
    newClass.addFunction(dtor);

    XSD::ComplexType pbcType = mWSDL.definitions().type().types().polymorphicBaseClass(*type);
    if (!pbcType.isNull()) {
        const QString pbc = mTypeMap.localType(pbcType.qualifiedName());
        KODE::Function clone("_kd_clone");
        clone.setConst(true);
        clone.setReturnType(pbc + QLatin1String(" *"));
        if (newClass.baseClasses().isEmpty()) {
            clone.setVirtualMode(KODE::Function::Virtual);
        } else {
            clone.setVirtualMode(KODE::Function::Override);
        }
        clone.setBody(QLatin1String("return new ") + newClassName + QLatin1String("(*this);"));
        newClass.addFunction(clone);

        const QName substElementName = type->substitutionElementName();
        if (!substElementName.isEmpty()) {
            KODE::Function substGetter("_kd_substitutionElementName");
            substGetter.setConst(true);
            substGetter.setReturnType(QLatin1String("QString"));
            if (newClass.baseClasses().isEmpty()) {
                substGetter.setVirtualMode(KODE::Function::Virtual);
            } else {
                substGetter.setVirtualMode(KODE::Function::Override);
            }
            substGetter.setBody(QString::fromLatin1("return QString::fromLatin1(\"%1\");").arg(substElementName.localName()));
            newClass.addFunction(substGetter);

            KODE::Function substNSGetter("_kd_substitutionElementNameSpace");
            substNSGetter.setConst(true);
            substNSGetter.setReturnType(QLatin1String("QString"));
            if (newClass.baseClasses().isEmpty()) {
                substNSGetter.setVirtualMode(KODE::Function::Virtual);
            } else {
                substNSGetter.setVirtualMode(KODE::Function::Override);
            }
            substNSGetter.setBody(QString::fromLatin1("return QString::fromLatin1(\"%1\");").arg(substElementName.nameSpace()));
            newClass.addFunction(substNSGetter);
        }
    }

    newClass.addHeaderInclude("QSharedPointer");
    mClasses.addClass(newClass);
}

// Called for each element and for each attribute of a complex type, as well as for the base class "value".
QString Converter::generateMemberVariable(const QString &rawName, const QString &typeName, const QString &inputTypeName, KODE::Class &newClass,
                                          XSD::Attribute::AttributeUse use, bool usePointer, bool polymorphic,
                                          const Converter::DefaultAttributeValue &defaultValue)
{
    const bool optional = (use == XSD::Attribute::Optional);
    const bool prohibited = (use == XSD::Attribute::Prohibited);
    const KODE::Function::AccessSpecifier access = (prohibited) ? KODE::Function::Private : KODE::Function::Public;

    // member variable
    const QString storageType = usePointer ? ElementArgumentSerializer::pointerStorageType(typeName) : typeName;
    KODE::MemberVariable variable(rawName, storageType);
    if (usePointer && !optional) {
        variable.setInitializer("new " + typeName);
    } else {
        addVariableInitializer(variable);
    }
    newClass.addMemberVariable(variable);

    KODE::MemberVariable nilVariable(rawName + "_nil", "bool");

    const QString variableName = QLatin1String("d_ptr->") + variable.name();
    const QString nilVariableName = QLatin1String("d_ptr->") + nilVariable.name();
    const QString upperName = upperlize(rawName);
    const QString lowerName = lowerlize(rawName);
    const QString memberName = mNameMapper.escape(lowerName);

    if (usePointer) {
        newClass.addInclude("QSharedPointer");

        // Give server-side implementations a way to dig into the received data
        KODE::MemberVariable variableAsSoapValue(rawName + "_as_kdsoap_value", "KDSoapValue");
        newClass.addMemberVariable(variableAsSoapValue);
        KODE::Function getterSoapValue(memberName + "_as_kdsoap_value", "KDSoapValue");
        getterSoapValue.setBody(QLatin1String("return d_ptr->") + variableAsSoapValue.name() + QLatin1Char(';'));
        getterSoapValue.setConst(true);
        newClass.addFunction(getterSoapValue);
    } else if (optional || prohibited) {
        nilVariable.setInitializer("true");
        newClass.addMemberVariable(nilVariable);
    }

    // setter method
    const QString argName = "arg_" + memberName;
    KODE::Function setter(QLatin1String("set") + upperName, QLatin1String("void"), access);
    setter.addArgument(inputTypeName + QLatin1Char(' ') + argName);
    KODE::Code code;

    if (usePointer) {
        if (polymorphic) {
            code += variableName + QLatin1String(" = ") + storageType + QLatin1Char('(') + argName + QLatin1String("._kd_clone());");
        } else {
            code += variableName + QLatin1String(" = ") + storageType + QLatin1String("(new ") + typeName + QLatin1Char('(') + argName
                + QLatin1String("));");
        }
    } else {
        if (optional) {
            code += nilVariableName + " = false;" + COMMENT;
        }
        code += variableName + QLatin1String(" = ") + argName + QLatin1Char(';');
    }
    setter.setBody(code);

    // getter method
    QString getterTypeName = typeName;
    if (optional && defaultValue.isNull()) {
        if (Settings::self()->optionalElementType() == Settings::EBoostOptional) {
            getterTypeName = "boost::optional<" + typeName + " >";
        } else if (Settings::self()->optionalElementType() == Settings::EStdOptional) {
            getterTypeName = "std::optional<" + typeName + " >";
        } else if (usePointer || Settings::self()->optionalElementType() == Settings::ERawPointer) {
            getterTypeName = "const " + typeName + QLatin1Char('*');
        }
    } else if (usePointer) {
        getterTypeName = QString("const " + typeName + '&');
    }
    KODE::Function getter(memberName, getterTypeName, access);
    if (usePointer) {
        if (optional) {
            if (Settings::self()->optionalElementType() == Settings::EBoostOptional) {
                KODE::Code getterCode;
                getterCode += QLatin1String("if (") + variableName + QLatin1String(")");
                getterCode.indent();
                getterCode += QLatin1String("return *") + variableName + QLatin1Char(';');
                getterCode.unindent();
                getterCode += QLatin1String("else");
                getterCode.indent();
                getterCode += "return boost::optional<" + typeName + " >();";
                getter.setBody(getterCode);
            } else if (Settings::self()->optionalElementType() == Settings::EStdOptional) {
                KODE::Code getterCode;
                getterCode += QLatin1String("if (") + variableName + QLatin1String(")");
                getterCode.indent();
                getterCode += QLatin1String("return *") + variableName + QLatin1Char(';');
                getterCode.unindent();
                getterCode += QLatin1String("else");
                getterCode.indent();
                getterCode += "return std::nullopt;";
                getter.setBody(getterCode);
            } else { // Regular isn't an option here. It would crash when the value is not set! So assume ERawPointer.
                getter.setBody(QLatin1String("return ") + variableName + QLatin1String(".data();"));
            }
        } else { // not optional -> API takes const ref
            getter.setBody(QLatin1String("return *") + variableName + QLatin1Char(';'));
        }
    } else if (optional) {
        if (Settings::self()->optionalElementType() == Settings::ERawPointer) {
            KODE::Code getterCode;
            getterCode += QLatin1String("if (!") + nilVariableName + QLatin1Char(')');
            getterCode.indent();
            if (defaultValue.isNull()) {
                getterCode += QLatin1String("return &") + variableName + QLatin1Char(';');
            } else {
                getterCode += QLatin1String("return ") + variableName + QLatin1Char(';');
            }
            getterCode.unindent();
            getterCode += QLatin1String("else");
            getterCode.indent();

            if (defaultValue.isNull()) {
                getterCode += "return 0;";
            } else {
                generateDefaultAttributeValueCode(getterCode, typeName, defaultValue);
            }
            getter.setBody(getterCode);
            getter.setDocs("Ownership is not transferred, clients shall not delete the pointer.");
        } else if (Settings::self()->optionalElementType() == Settings::EBoostOptional) {
            KODE::Code getterCode;
            getterCode += QLatin1String("if (!") + nilVariableName + QLatin1Char(')');
            getterCode.indent();
            getterCode += QLatin1String("return ") + variableName + QLatin1Char(';');
            getterCode.unindent();
            getterCode += QLatin1String("else");
            getterCode.indent();

            if (defaultValue.isNull()) {
                getterCode += "return boost::optional<" + typeName + " >();";
            } else {
                generateDefaultAttributeValueCode(getterCode, typeName, defaultValue);
            }

            getter.setBody(getterCode);
        } else if (Settings::self()->optionalElementType() == Settings::EStdOptional) {
            KODE::Code getterCode;
            getterCode += QLatin1String("if (!") + nilVariableName + QLatin1Char(')');
            getterCode.indent();
            getterCode += QLatin1String("return ") + variableName + QLatin1Char(';');
            getterCode.unindent();
            getterCode += QLatin1String("else");
            getterCode.indent();
            getterCode += "return std::nullopt;";
            getter.setBody(getterCode);
        } else {
            getter.setBody(QLatin1String("return ") + variableName + QLatin1Char(';'));
        }
    } else {
        getter.setBody(QLatin1String("return ") + variableName + QLatin1Char(';'));
    }
    getter.setConst(true);

    newClass.addFunction(setter);
    newClass.addFunction(getter);

    if (optional) {
        KODE::Code checkerCode;
        if (usePointer) {
            checkerCode += QLatin1String("return ") + variableName + QLatin1String(".isNull() == false;");
        } else {
            checkerCode += QLatin1String("return ") + nilVariableName + QLatin1String(" == false;");
        }

        KODE::Function checker(QLatin1String("hasValueFor") + upperName, QLatin1String("bool"), access);
        checker.setConst(true);
        checker.setBody(checkerCode);
        newClass.addFunction(checker);
    }

    return variableName;
}

// Helper method for the generation of the deserialize() method
static KODE::Code demarshalNameTest(const QName &type, const QString &tagName, bool *first)
{
    KODE::Code demarshalCode;
    if (type.nameSpace() == TypeMap::XMLSchemaURI() && (type.localName() == QLatin1String("any"))) {
        demarshalCode += QString::fromLatin1(*first ? "" : "else ") + QLatin1String("{") + COMMENT;
    } else {
        demarshalCode +=
            QString::fromLatin1(*first ? "" : "else ") + QLatin1String("if (_name == QLatin1String(\"") + tagName + QLatin1String("\")) {") + COMMENT;
    }
    *first = false;
    return demarshalCode;
}

void Converter::createComplexTypeSerializer(KODE::Class &newClass, const XSD::ComplexType *type)
{
    newClass.addInclude(QLatin1String("KDSoapClient/KDSoapNamespaceManager.h"));

    KODE::Function serializeFunc(QLatin1String("serialize"), QLatin1String("KDSoapValue"));
    serializeFunc.addArgument(QLatin1String("const QString& valueName"));
    if (!type->derivedTypes().isEmpty()) {
        serializeFunc.setVirtualMode(KODE::Function::Virtual);
    }
    if (!newClass.baseClasses().isEmpty()) {
        serializeFunc.setVirtualMode(KODE::Function::Override);
    }
    serializeFunc.setConst(true);

    KODE::Function deserializeFunc(QLatin1String("deserialize"), QLatin1String("void"));
    deserializeFunc.addArgument(QLatin1String("const KDSoapValue& mainValue"));
    if (!type->derivedTypes().isEmpty()) {
        deserializeFunc.setVirtualMode(KODE::Function::Virtual);
    }
    if (!newClass.baseClasses().isEmpty()) {
        deserializeFunc.setVirtualMode(KODE::Function::Override);
    }

    KODE::Code marshalCode, demarshalCode;

    const QString typeArgs = namespaceString(type->nameSpace()) + QLatin1String(", QString::fromLatin1(\"") + type->name() + QLatin1String("\")");

    if (type->baseTypeName() != XmlAnyType && !type->baseTypeName().isEmpty() && !type->isArray()) {

        const QName baseName = type->baseTypeName();

        const QString typeName = mTypeMap.localType(baseName);
        KODE::MemberVariable variable(QLatin1String("value"), typeName);
        const QString variableName = QLatin1String("d_ptr->") + variable.name();
        const QString nilVariableName = QLatin1String("d_ptr->") + variable.name() + "_nil";

        ElementArgumentSerializer serializer(mTypeMap, baseName, QName(), variableName, nilVariableName);

        if (mTypeMap.isComplexType(baseName)) {
            // This is different from ElementArgumentSerializer because we're calling BaseClass::serialize
            marshalCode += QLatin1String("KDSoapValue mainValue = ") + typeName + QLatin1String("::serialize(valueName);") + COMMENT;
            demarshalCode += typeName + "::deserialize(mainValue);";
        } else {
            if (mTypeMap.isBuiltinType(baseName)) {
                marshalCode += QLatin1String("KDSoapValue mainValue = ")
                    + mTypeMap.serializeBuiltin(baseName, QName(), variableName, "valueName", type->nameSpace(), type->name()) + QLatin1String(";")
                    + COMMENT;
            } else {
                marshalCode += QLatin1String("KDSoapValue mainValue = ") + variableName + QLatin1String(".serialize(valueName);") + COMMENT;
            }
            demarshalCode += serializer.demarshalVariable(QLatin1String("mainValue"));
        }

        if (!mTypeMap.isBuiltinType(baseName)) {
            marshalCode += QLatin1String("mainValue.setType(") + typeArgs + QLatin1String(");");
        }
    } else {
        marshalCode += QLatin1String("KDSoapValue mainValue(valueName, QVariant(), ") + typeArgs + QLatin1String(");") + COMMENT;
    }

    // elements
    XSD::Element::List elements = type->elements();

    // remove "void" elements (testcase: "result" in salesforce-partner's setPasswordResponse)
    QMutableListIterator<XSD::Element> itElem(elements);
    while (itElem.hasNext()) {
        const XSD::Element &elem = itElem.next();
        const QString typeName = mTypeMap.localType(elem.type());
        if (typeName == QLatin1String("void")) {
            itElem.remove();
        }
    }
    const XSD::Attribute::List attributes = type->attributes();
    if (!elements.isEmpty() || !attributes.isEmpty()) {
        demarshalCode += QLatin1String("const KDSoapValueList& args = mainValue.childValues();") + COMMENT;
    }

    if (!elements.isEmpty()) {
        marshalCode += QLatin1String("KDSoapValueList& args = mainValue.childValues();") + COMMENT;
        if (elements.at(0).isQualified()) {
            marshalCode += QLatin1String("mainValue.setQualified(true);") + COMMENT;
        }
        demarshalCode += "for (const KDSoapValue& val : qAsConst(args)) {";
        demarshalCode.indent();
        demarshalCode += "const QString _name = val.name();";
    } else {
        // The Q_UNUSED is not necessarily true in case of attributes, but who cares.
        demarshalCode += QLatin1String("Q_UNUSED(mainValue);") + COMMENT;
    }

    if (type->isArray()) {
        if (elements.count() != 1) {
            qDebug() << "array" << type->name() << "has" << elements.count() << "elements!";
        }
        Q_ASSERT(elements.count() == 1);
        const XSD::Element elem = elements.first();
        const QString variableName =
            QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(elem.name()); // always d_ptr->mEntries, actually
        const QString nilVariableName = QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(elem.name() + "_nil");
        const QName arrayType = type->arrayType();

        marshalCode += QLatin1String("args.setArrayType(QString::fromLatin1(\"") + arrayType.nameSpace()
            + QLatin1String("\"), QString::fromLatin1(\"") + arrayType.localName() + QLatin1String("\"));");
        marshalCode += QLatin1String("for (int i = 0; i < ") + variableName + QLatin1String(".count(); ++i) {") + COMMENT;
        marshalCode.indent();
        const QString nameSpace = elem.nameSpace();

        ElementArgumentSerializer serializer(mTypeMap, arrayType, QName(), variableName + QLatin1String(".at(i)"), QString());
        serializer.setElementName(QName(nameSpace, QLatin1String("item")));
        serializer.setOutputVariable("args", true);
        serializer.setIsQualified(elem.isQualified());
        serializer.setNillable(elem.nillable());
        serializer.setOptional(false); // if not set, not in array
        marshalCode.addBlock(serializer.generateSerializationCode());

        marshalCode.unindent();
        marshalCode += '}';

        ElementArgumentSerializer deserializer(mTypeMap, arrayType, QName(), variableName, nilVariableName);
        deserializer.setOptional(isElementOptional(elem));
        demarshalCode.addBlock(deserializer.demarshalArray("val"));
    } else {
        bool first = true;
        for (const XSD::Element &elem : qAsConst(elements)) {

            const QString elemName = elem.name();
            const QString typeName = mTypeMap.localType(elem.type());
            Q_ASSERT(!typeName.isEmpty());
            Q_ASSERT(typeName != QLatin1String("void")); // removed earlier in this file

            const QString variableName = QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(elemName);
            const QString nilVariableName = QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(elemName + "_nil");

            demarshalCode.addBlock(demarshalNameTest(elem.type(), elemName, &first));
            demarshalCode.indent();

            ElementArgumentSerializer serializer(mTypeMap, elem.type(), QName(), variableName, nilVariableName);
            serializer.setOutputVariable("args", true);
            serializer.setIsQualified(elem.isQualified());
            const QName qualName = elem.qualifiedName();

            if (elem.maxOccurs() > 1 || elem.compositor().maxOccurs() > 1) {
                // const QString typePrefix = mNSManager.prefix( elem.type().nameSpace() );

                QString localVariableName = variableName + QLatin1String(".at(i)");

                marshalCode += QLatin1String("for (int i = 0; i < ") + variableName + QLatin1String(".count(); ++i) {") + COMMENT;
                marshalCode.indent();

                serializer.setLocalVariableName(localVariableName);
                if (elem.hasSubstitutions())
                    serializer.setDynamicElementName(localVariableName + "->_kd_substitutionElementName()",
                                                     localVariableName + "->_kd_substitutionElementNameSpace()", qualName);
                else {
                    serializer.setElementName(qualName);
                }
                serializer.setOptional(false); // if not set, not in array

                marshalCode.addBlock(serializer.generateSerializationCode());
                marshalCode.unindent();
                marshalCode += '}';

                ElementArgumentSerializer deserializer(mTypeMap, elem.type(), QName(), variableName, nilVariableName);
                deserializer.setOptional(isElementOptional(elem));
                demarshalCode.addBlock(deserializer.demarshalArray("val"));
            } else {
                const bool optional = isElementOptional(elem);
                if (elem.hasSubstitutions())
                    serializer.setDynamicElementName(variableName + "->_kd_substitutionElementName()",
                                                     variableName + "->_kd_substitutionElementNameSpace()", qualName);
                else {
                    serializer.setElementName(qualName);
                }
                serializer.setOptional(optional);
                const bool usePointer = usePointerForElement(elem, newClass, mTypeMap, false);
                serializer.setUsePointer(usePointer);
                serializer.setNillable(elem.nillable());
                marshalCode.addBlock(serializer.generateSerializationCode());

                demarshalCode.addBlock(serializer.demarshalVariable("val"));
            }

            demarshalCode.unindent();
            demarshalCode += "}";
        } // end: for each element
    }

    if (!elements.isEmpty()) {
        demarshalCode.unindent();
        demarshalCode += "}";
    }

    if (!attributes.isEmpty()) {

        marshalCode += "KDSoapValueList attribs;";

        demarshalCode += "const QList<KDSoapValue> attribs = args.attributes();";
        demarshalCode += "for (const KDSoapValue& val : qAsConst(attribs)) {";
        demarshalCode.indent();
        demarshalCode += "const QString _name = val.name();";

        bool first = true;
        for (const XSD::Attribute &attribute : qAsConst(attributes)) {
            const QString attrName = attribute.name();
            if (attrName.isEmpty()) {
                continue;
            }
            const QString variableName = QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(attrName);
            const QString nilVariableName = QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(attrName + "_nil");

            demarshalCode.addBlock(demarshalNameTest(attribute.type(), attrName, &first));
            demarshalCode.indent();

            ElementArgumentSerializer serializer(mTypeMap, attribute.type(), QName(), variableName, nilVariableName);
            serializer.setElementName(attribute.qualifiedName());
            serializer.setOutputVariable("attribs", true);
            serializer.setIsQualified(attribute.isQualified());
            serializer.setNillable(false);
            serializer.setOptional(attribute.attributeUse() == XSD::Attribute::Optional || attribute.attributeUse() == XSD::Attribute::Prohibited);
            marshalCode.addBlock(serializer.generateSerializationCode());

            demarshalCode.addBlock(serializer.demarshalVariable("val"));

            demarshalCode.unindent();
            demarshalCode += "}";
        }
        marshalCode += QLatin1String("mainValue.childValues().attributes() += attribs;") + COMMENT;

        demarshalCode.unindent();
        demarshalCode += "}";
    }

    marshalCode += "return mainValue;";

    serializeFunc.setBody(marshalCode);
    newClass.addFunction(serializeFunc);

    deserializeFunc.setBody(demarshalCode);
    newClass.addFunction(deserializeFunc);
}
