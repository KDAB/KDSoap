#include "converter.h"
#include "settings.h"
#include "elementargumentserializer.h"
#include <libkode/style.h>

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
    if (!isList && isElementOptional(elem) && newClass.name() == typeName) {
        if (qgetenv("KDSOAP_TYPE_DEBUG").toInt()) {
            qDebug() << "Making element polymorphic:" << typeName << "in" << newClass.name();
        }
        // Optional "Foo" in class "Foo" - can't just have a value and bool, we need a pointer, to avoid infinite recursion
        //use == XSD::Attribute::Required;
        return true;
    }
    return polymorphic;
}

static QString pointerStorageType(const QString &typeName)
{
    if (typeName == "QString" || typeName == "bool") {
        qWarning() << "Should not happen: polymorphic" << typeName;
        Q_ASSERT(0);
    }

    return "QSharedPointer<" + typeName + '>';
}

void Converter::convertComplexType(const XSD::ComplexType *type)
{
    // An empty type is still useful, in document mode: it serializes the element name
    //if ( type->isEmpty() )
    //    return;

    // Skip the Array types we added in Parser::init...
    if (NSManager::soapEncNamespaces().contains(type->nameSpace())) {
        return;
    }

    const QString className(mTypeMap.localType(type->qualifiedName()));
    KODE::Class newClass;
    newClass.setNamespaceAndName(className);
    if (!Settings::self()->exportDeclaration().isEmpty()) {
        newClass.setExportDeclaration(Settings::self()->exportDeclaration());
    }

    newClass.setUseSharedData(true, QLatin1String("d_ptr") /*avoid clash with possible d() method */);

    const bool doDebug = (qgetenv("KDSOAP_TYPE_DEBUG").toInt());
    if (doDebug) {
        qDebug() << "Generating complex type" << className;
    }

    // subclass handling
    if (!type->baseTypeName().isEmpty()) {   // this class extends something
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
                const QString variableName = generateMemberVariable("value", typeName, inputTypeName, newClass, XSD::Attribute::Required, false, false);

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
    Q_FOREACH (const XSD::Element &elemIt, elements) {

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

        if (typeName != QLatin1String("void")) { // void means empty element, probably just here for later extensions (testcase: SetPasswordResult in salesforce)
            QString inputTypeName = mTypeMap.localInputType(elemIt.type(), QName());

            bool isList = false;
            if (elemIt.maxOccurs() > 1 || elemIt.compositor().maxOccurs() > 1) {
                QString itemType = mTypeMap.isPolymorphic(elemIt.type()) ? pointerStorageType(typeName) : typeName;
                typeName = listTypeFor(itemType, newClass);
                inputTypeName = QLatin1String("const ") + typeName + QLatin1String("&");
                isList = true;
            }
            if (type->isArray()) {
                QString arrayTypeName = mTypeMap.localType(type->arrayType());
                Q_ASSERT(!arrayTypeName.isEmpty());
                if (mTypeMap.isPolymorphic(type->arrayType())) {
                    arrayTypeName = pointerStorageType(arrayTypeName);
                }
                //qDebug() << "array of" << attribute.arrayType() << "->" << arrayTypeName;
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
    XSD::Attribute::List attributes = type->attributes();
    Q_FOREACH (const XSD::Attribute &attribute, attributes) {
        QString typeName, inputTypeName;

        typeName = mTypeMap.localType(attribute.type());
        if (typeName.isEmpty()) {
            qDebug() << "ERROR: attribute with unknown type:" << attribute.name() << attribute.type() << "in" << typeName;
        }
        inputTypeName = mTypeMap.localInputType(attribute.type(), QName());
        //qDebug() << "Attribute" << attribute.name();

        generateMemberVariable(KODE::Style::makeIdentifier(attribute.name()), typeName, inputTypeName, newClass, attribute.attributeUse(), false, false);

        // include header
        newClass.addIncludes(QStringList(), mTypeMap.forwardDeclarations(attribute.type()));
        newClass.addHeaderIncludes(mTypeMap.headerIncludes(attribute.type()));
    }

    createComplexTypeSerializer(newClass, type);

    const QString newClassName = newClass.name();

    KODE::Function ctor(/*upperlize*/(newClassName));
    newClass.addFunction(ctor);

    KODE::Function dtor(QLatin1Char('~') + /*upperlize*/(newClassName));
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
        clone.setVirtualMode(KODE::Function::Virtual);
        clone.setBody(QLatin1String("return new ") + newClassName + QLatin1String("(*this);"));
        newClass.addFunction(clone);

        const QName substElementName = type->substitutionElementName();
        if (!substElementName.isEmpty()) {
            KODE::Function substGetter("_kd_substitutionElementName");
            substGetter.setConst(true);
            substGetter.setReturnType(QLatin1String("QString"));
            substGetter.setVirtualMode(KODE::Function::Virtual);
            substGetter.setBody(QString::fromLatin1("return QString::fromLatin1(\"%1\");").arg(substElementName.localName()));
            newClass.addFunction(substGetter);

            KODE::Function substNSGetter("_kd_substitutionElementNameSpace");
            substNSGetter.setConst(true);
            substNSGetter.setReturnType(QLatin1String("QString"));
            substNSGetter.setVirtualMode(KODE::Function::Virtual);
            substNSGetter.setBody(QString::fromLatin1("return QString::fromLatin1(\"%1\");").arg(substElementName.nameSpace()));
            newClass.addFunction(substNSGetter);
        }
    }

    newClass.addInclude("QSharedPointer");
    mClasses.addClass(newClass);
}

// Called for each element and for each attribute of a complex type, as well as for the base class "value".
QString Converter::generateMemberVariable(const QString &rawName, const QString &typeName, const QString &inputTypeName, KODE::Class &newClass, XSD::Attribute::AttributeUse use, bool usePointer, bool polymorphic)
{
    // member variable
    const QString storageType = usePointer ? pointerStorageType(typeName) : typeName;
    KODE::MemberVariable variable(rawName, storageType);
    addVariableInitializer(variable);
    newClass.addMemberVariable(variable);

    const QString variableName = QLatin1String("d_ptr->") + variable.name();
    const QString upperName = upperlize(rawName);
    const QString lowerName = lowerlize(rawName);
    const QString memberName = mNameMapper.escape(lowerName);

    bool optional = (use == XSD::Attribute::Optional);
    bool prohibited = (use == XSD::Attribute::Prohibited);
    const KODE::Function::AccessSpecifier access = (prohibited) ? KODE::Function::Private : KODE::Function::Public;

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
        KODE::MemberVariable nilVariable(rawName + "_nil", "bool");
        nilVariable.setInitializer("true");
        newClass.addMemberVariable(nilVariable);
    }

    // setter method
    const QString argName = '_' + memberName;
    KODE::Function setter(QLatin1String("set") + upperName, QLatin1String("void"), access);
    setter.addArgument(inputTypeName + QLatin1Char(' ') + argName);
    KODE::Code code;

    if (usePointer) {
        if (polymorphic) {
            code += variableName + QLatin1String(" = ") + storageType + QLatin1Char('(') + argName + QLatin1String("._kd_clone());");
        } else {
            code += variableName + QLatin1String(" = ") + storageType + QLatin1String("(new ") + typeName + QLatin1Char('(') + argName + QLatin1String("));");
        }
    } else {
        if (optional) {
            code += variableName + "_nil = false;" + COMMENT;
        }
        code += variableName + QLatin1String(" = ") + argName + QLatin1Char(';');
    }
    setter.setBody(code);

    // getter method
    QString getterTypeName = typeName;
    if (optional) {
        if (Settings::self()->optionalElementType() == Settings::EBoostOptional) {
            getterTypeName = "boost::optional<" + typeName + " >";
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
            } else { // Regular isn't an option here. It would crash when the value is not set! So assume ERawPointer.
                getter.setBody(QLatin1String("return ") + variableName + QLatin1String(".data();"));
            }
        } else { // not optional -> API takes const ref
            getter.setBody(QLatin1String("return *") + variableName + QLatin1Char(';'));
        }
    } else if (optional) {
        if (Settings::self()->optionalElementType() == Settings::ERawPointer) {
            KODE::Code getterCode;
            getterCode += QLatin1String("if (!") + variableName + QLatin1String("_nil)");
            getterCode.indent();
            getterCode += QLatin1String("return &") + variableName + QLatin1Char(';');
            getterCode.unindent();
            getterCode += QLatin1String("else");
            getterCode.indent();
            getterCode += "return 0;";
            getter.setBody(getterCode);
            getter.setDocs("Ownership is not transferred, clients shall not delete the pointer.");
        } else if (Settings::self()->optionalElementType() == Settings::EBoostOptional) {
            KODE::Code getterCode;
            getterCode += QLatin1String("if (!") + variableName + QLatin1String("_nil)");
            getterCode.indent();
            getterCode += QLatin1String("return ") + variableName + QLatin1Char(';');
            getterCode.unindent();
            getterCode += QLatin1String("else");
            getterCode.indent();
            getterCode += "return boost::optional<" + typeName + " >();";
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

    return variableName;
}

// Helper method for the generation of the deserialize() method
static KODE::Code demarshalNameTest(const QName &type, const QString &tagName, bool *first)
{
    KODE::Code demarshalCode;
    if (type.nameSpace() == XMLSchemaURI && (type.localName() == QLatin1String("any"))) {
        demarshalCode += QString::fromLatin1(*first ? "" : "else ") + QLatin1String("{") + COMMENT;
    } else {
        demarshalCode += QString::fromLatin1(*first ? "" : "else ") + QLatin1String("if (_name == QLatin1String(\"") + tagName + QLatin1String("\")) {") + COMMENT;
    }
    *first = false;
    return demarshalCode;
}

// Low-level helper for demarshalVar, doesn't handle the polymorphic case (so it can be called for lists of polymorphics)
KODE::Code Converter::demarshalVarHelper(const QName &type, const QName &elementType, const QString &variableName, const QString &qtTypeName, const QString &soapValueVarName, bool optional) const
{
    KODE::Code code;
    if (mTypeMap.isTypeAny(type)) {
        code += variableName + QLatin1String(" = ") + soapValueVarName + QLatin1String(";") + COMMENT;
    } else if (mTypeMap.isBuiltinType(type, elementType)) {
        code += variableName + QLatin1String(" = ") + mTypeMap.deserializeBuiltin(type, elementType, soapValueVarName + QLatin1String(".value()"), qtTypeName) + QLatin1String(";") + COMMENT;
    } else if (mTypeMap.isComplexType(type, elementType)) {
        code += variableName + QLatin1String(".deserialize(") + soapValueVarName + QLatin1String(");") + COMMENT;
    } else {
        code += variableName + QLatin1String(".deserialize(") + soapValueVarName + QLatin1String(".value());") + COMMENT;
    }
    if (optional) {
        code += variableName + QLatin1String("_nil = false;") + COMMENT;
    }
    return code;
}

// Helper method for the generation of the deserialize() method, also used by convertClientCall
KODE::Code Converter::demarshalVar(const QName &type, const QName &elementType, const QString &variableName, const QString &qtTypeName, const QString &soapValueVarName, bool optional, bool usePointer) const
{
    const bool isPolymorphic = mTypeMap.isPolymorphic(type, elementType);
    if (usePointer || isPolymorphic) {
        const QString storageType = pointerStorageType(qtTypeName);
        KODE::Code code;
        code += variableName + "_as_kdsoap_value = " + soapValueVarName + ";" + COMMENT;
        code += "if (!" + variableName + ")";
        code.indent();
        code += variableName + " = " + storageType + "(new " + qtTypeName + ");" + COMMENT;
        code.unindent();
        code += variableName + QLatin1String("->deserialize(") + soapValueVarName + QLatin1String(");") + COMMENT;
        return code;
    } else {
        return demarshalVarHelper(type, elementType, variableName, qtTypeName, soapValueVarName, optional);
    }
}

KODE::Code Converter::demarshalArrayVar(const QName &type, const QString &variableName, const QString &qtTypeName) const
{
    KODE::Code code;
    if (mTypeMap.isTypeAny(type)) {     // KDSoapValue doesn't support temp vars [still true?]. This special-casing is ugly though.
        code += variableName + QLatin1String(".append(val);");
    } else {
        // we need a temp var in case of deserialize()
        // [TODO: we could merge demarshalVar into this code, to avoid the temp var in other cases]
        QString tempVar;
        if (variableName.startsWith(QLatin1String("d_ptr->"))) {
            tempVar = variableName.mid(7) + QLatin1String("Temp");
        } else {
            tempVar = variableName + QLatin1String("Temp");
        }
        code += qtTypeName + QLatin1String(" ") + tempVar + QLatin1String(";") + COMMENT;
        code.addBlock(demarshalVarHelper(type, QName(), tempVar, qtTypeName, "val", false));
        QString toAppend = tempVar;
        const bool isPolymorphic = mTypeMap.isPolymorphic(type);
        if (isPolymorphic) {
            const QString storageType = pointerStorageType(qtTypeName);
            toAppend = storageType + "(new " + qtTypeName + "(" + tempVar + "))";
        }
        code += variableName + QLatin1String(".append(") + toAppend + QLatin1String(");") + COMMENT;
    }
    return code;
}

void Converter::createComplexTypeSerializer(KODE::Class &newClass, const XSD::ComplexType *type)
{
    newClass.addInclude(QLatin1String("KDSoapClient/KDSoapNamespaceManager.h"));

    KODE::Function serializeFunc(QLatin1String("serialize"), QLatin1String("KDSoapValue"));
    serializeFunc.addArgument(QLatin1String("const QString& valueName"));
    if (!type->derivedTypes().isEmpty()) {
        serializeFunc.setVirtualMode(KODE::Function::Virtual);
    }
    serializeFunc.setConst(true);

    KODE::Function deserializeFunc(QLatin1String("deserialize"), QLatin1String("void"));
    deserializeFunc.addArgument(QLatin1String("const KDSoapValue& mainValue"));
    if (!type->derivedTypes().isEmpty()) {
        deserializeFunc.setVirtualMode(KODE::Function::Virtual);
    }

    KODE::Code marshalCode, demarshalCode;

    const QString typeArgs = namespaceString(type->nameSpace()) + QLatin1String(", QString::fromLatin1(\"") + type->name() + QLatin1String("\")");

    if (type->baseTypeName() != XmlAnyType && !type->baseTypeName().isEmpty() && !type->isArray()) {

        const QName baseName = type->baseTypeName();
        const QString typeName = mTypeMap.localType(baseName);
        KODE::MemberVariable variable(QLatin1String("value"), typeName);
        const QString variableName = QLatin1String("d_ptr->") + variable.name();

        if (mTypeMap.isComplexType(baseName)) {
            //marshalCode += QLatin1String("KDSoapValue mainValue = ") + variableName + QLatin1String(".serialize(valueName);") + COMMENT;
            //marshalCode += QLatin1String("mainValue.setType(") + typeArgs + QLatin1String(");");
            marshalCode += QLatin1String("KDSoapValue mainValue = ") + typeName + QLatin1String("::serialize(valueName);") + COMMENT;
            marshalCode += QLatin1String("mainValue.setType(") + typeArgs + QLatin1String(");");
            //demarshalCode += demarshalVar( baseName, QName(), variableName, typeName, QLatin1String("mainValue") );

            demarshalCode += typeName + "::deserialize(mainValue);";
            //demarshalCode += demarshalVar( baseName, QName(), variableName, typeName, QLatin1String("mainValue") );
        } else {
            QString value;
            if (mTypeMap.isBuiltinType(baseName)) {
                value = mTypeMap.serializeBuiltin(baseName, QName(), variableName, typeName);
            } else {
                value += variableName + QLatin1String(".serialize()");
            }
            marshalCode += QLatin1String("KDSoapValue mainValue(valueName, ") + value + QLatin1String(", ") + typeArgs + QLatin1String(");") + COMMENT;
            demarshalCode += demarshalVar(baseName, QName(), variableName, typeName, QLatin1String("mainValue"), false, false);
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
        demarshalCode += "for (int argNr = 0; argNr < args.count(); ++argNr) {";
        demarshalCode.indent();
        demarshalCode += "const KDSoapValue& val = args.at(argNr);";
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
        const QString variableName = QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(elem.name());   // always d_ptr->mEntries, actually
        const QName arrayType = type->arrayType();
        const QString typeName = mTypeMap.localType(arrayType);

        marshalCode += QLatin1String("args.setArrayType(QString::fromLatin1(\"") + arrayType.nameSpace() + QLatin1String("\"), QString::fromLatin1(\"") + arrayType.localName() + QLatin1String("\"));");
        marshalCode += QLatin1String("for (int i = 0; i < ") + variableName + QLatin1String(".count(); ++i) {") + COMMENT;
        marshalCode.indent();
        const QString nameSpace = elem.nameSpace();

        ElementArgumentSerializer serializer(mTypeMap, arrayType, QName(), variableName + QLatin1String(".at(i)"));
        serializer.setElementName(QName(nameSpace, QLatin1String("item")));
        serializer.setOutputVariable("args", true);
        serializer.setIsQualified(elem.isQualified());
        serializer.setNillable(elem.nillable());
        serializer.setOmitIfEmpty(false);   // if not set, not in array
        marshalCode.addBlock(serializer.generate());

        marshalCode.unindent();
        marshalCode += '}';

        demarshalCode.addBlock(demarshalArrayVar(arrayType, variableName, typeName));
    } else {
        bool first = true;
        Q_FOREACH (const XSD::Element &elem, elements) {

            const QString elemName = elem.name();
            const QString typeName = mTypeMap.localType(elem.type());
            Q_ASSERT(!typeName.isEmpty());
            Q_ASSERT(typeName != QLatin1String("void")); // removed earlier in this file

            const QString variableName = QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(elemName);

            demarshalCode.addBlock(demarshalNameTest(elem.type(), elemName, &first));
            demarshalCode.indent();

            ElementArgumentSerializer serializer(mTypeMap, elem.type(), QName(), variableName);
            serializer.setOutputVariable("args", true);
            serializer.setIsQualified(elem.isQualified());
            serializer.setNillable(elem.nillable());
            const QName qualName = elem.qualifiedName();

            if (elem.maxOccurs() > 1 || elem.compositor().maxOccurs() > 1) {
                //const QString typePrefix = mNSManager.prefix( elem.type().nameSpace() );

                QString localVariableName = variableName + QLatin1String(".at(i)");

                marshalCode += QLatin1String("for (int i = 0; i < ") + variableName + QLatin1String(".count(); ++i) {") + COMMENT;
                marshalCode.indent();

                serializer.setLocalVariableName(localVariableName);
                if (elem.hasSubstitutions())
                    serializer.setDynamicElementName(localVariableName + "->_kd_substitutionElementName()",
                                                     localVariableName + "->_kd_substitutionElementNameSpace()",
                                                     qualName);
                else {
                    serializer.setElementName(qualName);
                }
                serializer.setOmitIfEmpty(false);   // if not set, not in array

                marshalCode.addBlock(serializer.generate());
                marshalCode.unindent();
                marshalCode += '}';

                demarshalCode.addBlock(demarshalArrayVar(elem.type(), variableName, typeName));
            } else {
                const bool optional = isElementOptional(elem);
                if (elem.hasSubstitutions())
                    serializer.setDynamicElementName(variableName + "->_kd_substitutionElementName()",
                                                     variableName + "->_kd_substitutionElementNameSpace()",
                                                     qualName);
                else {
                    serializer.setElementName(qualName);
                }
                serializer.setOmitIfEmpty(optional);
                const bool usePointer = usePointerForElement(elem, newClass, mTypeMap, false);
                serializer.setUsePointer(usePointer);
                marshalCode.addBlock(serializer.generate());

                demarshalCode.addBlock(demarshalVar(elem.type(), QName(), variableName, typeName, "val", optional, usePointer));
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
        demarshalCode += "for (int attrNr = 0; attrNr < attribs.count(); ++attrNr) {";
        demarshalCode.indent();
        demarshalCode += "const KDSoapValue& val = attribs.at(attrNr);";
        demarshalCode += "const QString _name = val.name();";

        bool first = true;
        Q_FOREACH (const XSD::Attribute &attribute, attributes) {
            const QString attrName = attribute.name();
            const QString variableName = QLatin1String("d_ptr->") + KODE::MemberVariable::memberVariableName(attrName);

            demarshalCode.addBlock(demarshalNameTest(attribute.type(), attrName, &first));
            demarshalCode.indent();

            ElementArgumentSerializer serializer(mTypeMap, attribute.type(), QName(), variableName);
            serializer.setElementName(attribute.qualifiedName());
            serializer.setOutputVariable("attribs", true);
            serializer.setIsQualified(attribute.isQualified());
            serializer.setNillable(false);
            serializer.setOmitIfEmpty(attribute.attributeUse() == XSD::Attribute::Optional || attribute.attributeUse() == XSD::Attribute::Prohibited);
            marshalCode.addBlock(serializer.generate());

            const QString typeName = mTypeMap.localType(attribute.type());
            Q_ASSERT(!typeName.isEmpty());
            demarshalCode.addBlock(demarshalVar(attribute.type(), QName(), variableName, typeName, "val", attribute.attributeUse() == XSD::Attribute::Optional, false));

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
