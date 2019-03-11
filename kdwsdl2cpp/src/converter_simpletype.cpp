#include "converter.h"
#include "settings.h"
#include <libkode/style.h>

#include <QDebug>

using namespace KWSDL;

static QString escapeEnum(const QString &);
static KODE::Code createRangeCheckCode(const XSD::SimpleType *, const QString &baseTypeName, const QString &, KODE::Class &, const XSD::SimpleType &);

void Converter::addVariableInitializer(KODE::MemberVariable &variable) const
{
    const QByteArray type = variable.type().toLatin1();
    static const char *s_numericTypes[] = {
        "int", "unsigned int", "quint64", "qint64",
        "char", "signed char", "unsigned char", "short", "unsigned short",
        "float", "double"
    };
    for (uint i = 0; i < sizeof(s_numericTypes) / sizeof(*s_numericTypes); ++i) {
        if (type == s_numericTypes[i]) {
            variable.setInitializer("0");
            return;
        }
    }
    if (type == "bool") {
        variable.setInitializer("false");
    }
}

QString Converter::listTypeFor(const QString &itemTypeName, KODE::Class &newClass)
{
    if (itemTypeName == QLatin1String("QString")) {
        newClass.addHeaderInclude("QtCore/QStringList");
        return "QStringList";
    }
    return "QList< " + itemTypeName + " >";
}

// Overall logic:
// if ENUM -> define "Type" and "type" variable
// if restricts a basic type or another simple type -> "value" variable
// else if list -> define a QList

void Converter::convertSimpleType(const XSD::SimpleType *type, const XSD::SimpleType::List &simpleTypeList)
{
    const QString typeName(mTypeMap.localType(type->qualifiedName()));
    //qDebug() << "convertSimpleType:" << type->qualifiedName() << typeName;
    KODE::Class newClass;
    newClass.setNamespaceAndName(typeName);
    if (!Settings::self()->exportDeclaration().isEmpty()) {
        newClass.setExportDeclaration(Settings::self()->exportDeclaration());
    }
    newClass.setNameSpace(Settings::self()->nameSpace());

    QString classDocumentation;

    switch (type->subType()) {
    case XSD::SimpleType::TypeRestriction: {
        /**
          Use setter and getter method for enums as well.
         */
        if (type->facetType() & XSD::SimpleType::ENUM) {
            classDocumentation = "This class is a wrapper for an enumeration.\n";
            NameMapper nameMapper;
            QStringList enums = type->facetEnums();
            for (int i = 0; i < enums.count(); ++i) {
                enums[ i ] = nameMapper.escape(escapeEnum(enums[ i ]));
            }

            newClass.addEnum(KODE::Enum("Type", enums));

            classDocumentation += "Whenever you have to pass an object of type " + newClass.name() +
                                  " you can also pass the enum directly. Example:\n" +
                                  "someMethod(" + newClass.name() + "::" + enums.first() + ").";

            // member variables
            KODE::MemberVariable variable("type", "Type");
            variable.setInitializer("Type(0)");
            newClass.addMemberVariable(variable);

            // setter method
            KODE::Function setter("setType", "void");
            setter.addArgument("Type type");
            setter.setBody(variable.name() + " = type;");

            // getter method
            KODE::Function getter("type", newClass.qualifiedName() + "::Type");
            getter.setBody("return " + variable.name() + ';');
            getter.setConst(true);

            // convenience constructor
            KODE::Function conctor(newClass.name());
            conctor.addArgument("const Type &type");
            KODE::Code code;
            code += variable.name() + " = type;";
            conctor.setBody(code);

            // type operator
            KODE::Function op("operator Type");
            op.setBody("return " + variable.name() + ';');
            op.setConst(true);

            newClass.addFunction(conctor);
            newClass.addFunction(setter);
            newClass.addFunction(getter);
            newClass.addFunction(op);
        }

        /**
          A class can't derive from basic types (e.g. int or unsigned char), so
          we add setter and getter methods to set the value of this class.
         */
        if (type->baseTypeName() != XmlAnyType
                && !type->baseTypeName().isEmpty()
                && !(type->facetType() & XSD::SimpleType::ENUM)) {
            classDocumentation = "This class encapsulates a simple type.\n";

            const QName baseName = type->baseTypeName();
            const QString baseTypeName = mTypeMap.localType(baseName);
            Q_ASSERT(!baseTypeName.isEmpty());

            QList<QName> parentBasicTypes;
            parentBasicTypes.append(baseName);
            QName currentType = baseName;
            Q_FOREVER {
                const XSD::SimpleType simpleType = simpleTypeList.simpleType(currentType);
                if (!simpleType.isNull() && simpleType.isRestriction()) {
                    currentType = simpleType.baseTypeName();
                    parentBasicTypes.append(currentType);
                    continue;
                }
                break;
            }

            classDocumentation += "Whenever you have to pass an object of type " + newClass.name() +
                                  " you can also pass the value directly as a " + mTypeMap.localType(currentType) + '.';
            // include header
            newClass.addIncludes(QStringList(), mTypeMap.forwardDeclarations(baseName));
            newClass.addHeaderIncludes(mTypeMap.headerIncludes(baseName));

            // member variables
            KODE::MemberVariable variable("value", baseTypeName);
            addVariableInitializer(variable);
            newClass.addMemberVariable(variable);

            // setter method
            KODE::Function setter("setValue", "void");
            const QString inputType = mTypeMap.localInputType(baseName, QName());
            setter.addArgument(inputType + " value");
            KODE::Code setterBody;
            if (type->facetType() != XSD::SimpleType::NONE) {
                const XSD::SimpleType baseSimpleType = simpleTypeList.simpleType(baseName);
                setterBody += createRangeCheckCode(type, baseTypeName, "value", newClass, baseSimpleType);
                setterBody.newLine();
                setterBody += "if (!rangeOk)";
                setterBody.indent();
                setterBody += "qDebug( \"Invalid range in " + newClass.name() + "::" + setter.name() + "()\" );";
                setterBody.unindent();
                setterBody.newLine();
            }
            setterBody += variable.name() + " = value;"; // ### call setValue in base class?
            setter.setBody(setterBody);
            newClass.addFunction(setter);

            // getter method
            KODE::Function getter("value", baseTypeName);
            getter.setBody("return " + variable.name() + ';');
            getter.setConst(true);
            newClass.addFunction(getter);

            // convenience constructor
            KODE::Function conctor(newClass.name());
            conctor.addArgument(inputType + " value");
            conctor.addBodyLine("setValue(value);");
            newClass.addFunction(conctor);

            // even more convenient constructor, for the case of multiple-level simple-type restrictions
            //qDebug() << typeName << ": baseName=" << baseName << "further up:" << parentBasicTypes;
            if (parentBasicTypes.count() > 1) {
                parentBasicTypes.removeLast(); // the top-most one is in "currentType", so it's the input arg.
                KODE::Function baseCtor(conctor.name());
                baseCtor.addArgument(mTypeMap.localInputType(currentType, QName()) + " value");
                QString beginLine = "setValue(";
                QString endLine = ")";
                Q_FOREACH (const QName &base, parentBasicTypes) {
                    beginLine += mTypeMap.localType(base) + '(';
                    endLine += ')';
                }
                baseCtor.addBodyLine(beginLine + "value" + endLine + ';');
                newClass.addFunction(baseCtor);
            }

            // type operator
            KODE::Function op("operator " + baseTypeName);
            op.setBody("return " + variable.name() + ';');
            op.setConst(true);
            newClass.addFunction(op);
        }
    }
    break;
    case XSD::SimpleType::TypeList: {
        classDocumentation = "This class encapsulates a list type.";

        newClass.addHeaderInclude("QtCore/QList");
        const QName baseName = type->listTypeName();
        const QString itemTypeName = mTypeMap.localType(baseName);
        Q_ASSERT(!itemTypeName.isEmpty());

        // include header
        newClass.addIncludes(QStringList(), mTypeMap.forwardDeclarations(baseName));
        newClass.addHeaderIncludes(mTypeMap.headerIncludes(baseName));

        const QString listType = listTypeFor(itemTypeName, newClass);

        // member variables
        KODE::MemberVariable variable("entries", listType);
        newClass.addMemberVariable(variable);

        // setter method
        KODE::Function setter("setEntries", "void");
        setter.addArgument("const " + listType + "& entries");
        setter.setBody(variable.name() + " = entries;");

        // getter method
        KODE::Function getter("entries", listType);
        getter.setBody("return " + variable.name() + ';');
        getter.setConst(true);

        newClass.addFunction(setter);
        newClass.addFunction(getter);

    }
    break;
    case XSD::SimpleType::TypeUnion:
        classDocumentation = "This class encapsulates a union type.";
        // Let's be lazy and let's just have a qvariant for storing the various
        // possible simple types (int, QString).
        newClass.addHeaderInclude("QtCore/QVariant");

        KODE::MemberVariable variable("value", "QVariant");
        newClass.addMemberVariable(variable);

        // setter method
        KODE::Function setter("setValue", "void");
        setter.addArgument("const QVariant& val");
        setter.setBody(variable.name() + " = val;");

        // getter method
        KODE::Function getter("value", "QVariant");
        getter.setBody("return " + variable.name() + ';');
        getter.setConst(true);

        newClass.addFunction(setter);
        newClass.addFunction(getter);

        break;
    };

    if (!type->documentation().isEmpty()) {
        newClass.setDocs(type->documentation().simplified());
    } else {
        newClass.setDocs(classDocumentation);
    }

    createSimpleTypeSerializer(newClass, type, simpleTypeList);

    // Empty ctor. Needed for derived simpleTypes (which use this one as value).
    KODE::Function emptyCtor(newClass.name());
    newClass.addFunction(emptyCtor);

    // Empty dtor. Just in case ;)
    KODE::Function dtor('~' + newClass.name());
    newClass.addFunction(dtor);

    mClasses.addClass(newClass);
}

void Converter::createSimpleTypeSerializer(KODE::Class &newClass, const XSD::SimpleType *type, const XSD::SimpleType::List &simpleTypeList)
{
    const QString typeName = mTypeMap.localType(type->qualifiedName());

    KODE::Function serializeFunc(QLatin1String("serialize"), QLatin1String("KDSoapValue"));
    serializeFunc.addArgument(QLatin1String("const QString& valueName"));
    serializeFunc.setConst(true);

    KODE::Function deserializeFunc("deserialize", "void");
    deserializeFunc.addArgument("const KDSoapValue& mainValue");

    switch (type->subType()) {
    case XSD::SimpleType::TypeRestriction:
        // is an enumeration
        if (type->facetType() & XSD::SimpleType::ENUM) {
            const QStringList enums = type->facetEnums();
            NameMapper nameMapper;
            QStringList escapedEnums;
            for (int i = 0; i < enums.count(); ++i) {
                escapedEnums.append(nameMapper.escape(escapeEnum(enums[ i ])));
            }

            const QString variableName = KODE::MemberVariable::memberVariableName("type");

            {
                KODE::Code code;
                code += "switch ( " + variableName + " ) {";
                code.indent();
                for (int i = 0; i < enums.count(); ++i) {
                    code += "case " + typeName + "::" + escapedEnums[ i ] + ':';
                    code.indent();
                    code += "return KDSoapValue(valueName, \"" + enums[i] + "\", " + namespaceString(type->nameSpace()) + ", QString::fromLatin1(\"" + type->name() + "\"));";
                    code.unindent();
                    /* add a hack for msvc because that one cannot parse switch statements
                       longer than a certain length, so start a new switch statement */
                    if (i % 64 == 63) {
                        code += "default:"; // silence gcc
                        code += "break;";
                        code.unindent();
                        code += '}';
                        code.newLine();
                        code += "switch ( " + variableName + " ) {";
                        code.indent();
                    }
                }
                code += "default:";
                code.indent();
                code += "qDebug(\"Unknown enum %d passed.\", " + variableName + ");";
                code += "break;";
                code.unindent();
                code.unindent();
                code += '}';
                code.newLine();
                code += "return KDSoapValue();";
                serializeFunc.setBody(code);
            }
            {
                KODE::Code code;
                code += "static const struct { const char* name; Type value; } s_values[" + QString::number(enums.count()) + "] = {";
                for (int i = 0; i < enums.count(); ++i) {
                    code += "{ \"" + enums[ i ] + "\", " + typeName + "::" + escapedEnums[ i ] + " }" + (i < enums.count() - 1 ? "," : "");
                }
                code += "};";
                code += "const QString str = mainValue.value().toString();";
                code += "for ( int i = 0; i < " + QString::number(enums.count()) + "; ++i ) {";
                code.indent();
                code += "if (str == QLatin1String(s_values[i].name)) {";
                code.indent();
                code += variableName + " = s_values[i].value;";
                code += "return;";
                code.unindent();
                code += "}";
                code.unindent();
                code += "}";
                code += "qDebug(\"Unknown enum value '%s' passed to '" + newClass.name() + "'.\", qPrintable(str) );";
                deserializeFunc.setBody(code);
            }

        }
        if (type->baseTypeName() != XmlAnyType
                && !type->baseTypeName().isEmpty()
                && !(type->facetType() & XSD::SimpleType::ENUM)) {
            // 'inherits' a basic type or another simple type -> using value.

            const QName baseName = type->baseTypeName();
            const QString baseTypeName = mTypeMap.localType(baseName);
            Q_ASSERT(!baseTypeName.isEmpty());
            const QString variableName = KODE::MemberVariable::memberVariableName("value");
            const QName baseType = type->baseTypeName();
            Q_UNUSED(simpleTypeList);
            //const QName mostBasicTypeName = simpleTypeList.mostBasicType( baseType );
            //Q_UNUSED(mostBasicTypeName);
            if (mTypeMap.isBuiltinType(baseType)) {     // serialize from QString, int, etc.
                serializeFunc.addBodyLine("return " + mTypeMap.serializeBuiltin(baseType, QName(), variableName, "valueName", type->nameSpace(), type->name()) + ";" + COMMENT);
                deserializeFunc.addBodyLine(variableName + " = " + mTypeMap.deserializeBuiltin(baseType, QName(), "mainValue", baseTypeName) + ";" + COMMENT);
            } else { // inherits another simple type, need to call its serialize/deserialize method
                serializeFunc.addBodyLine("KDSoapValue value = mValue.serialize(valueName);");
                serializeFunc.addBodyLine("value.setType(" + namespaceString(type->nameSpace()) + ", QString::fromLatin1(\"" + type->name() + "\"));");
                serializeFunc.addBodyLine("return value;" + COMMENT);
                deserializeFunc.addBodyLine(variableName + ".deserialize( mainValue );" + COMMENT);
            }

        }
        break;
    case XSD::SimpleType::TypeList: {
        const QName baseName = type->listTypeName();
        const QString itemTypeName = mTypeMap.localType(baseName);
        const QString variableName = KODE::MemberVariable::memberVariableName("entries");
        {
            KODE::Code code;
            code += "QString str;";
            code += "QXmlStreamNamespaceDeclarations decls;";
            code += "for ( int i = 0; i < " + variableName + ".count(); ++i ) {";
            code.indent();
            code += "if (!str.isEmpty())";
            code.indent();
            code += "str += QLatin1Char(' ');";
            code.unindent();
            if (itemTypeName == "QString") { // special but common case, no conversion needed
                code += "str += " + variableName + ".at(i);";
            } else {
                if (mTypeMap.isBuiltinType(baseName)) { // serialize from int, float, bool, etc.
                    code += "KDSoapValue subValue = " + mTypeMap.serializeBuiltin(baseName, QName(), variableName + ".at(i)", "QString()", QString(), QString()) + ";";
                } else {
                    code += "KDSoapValue subValue =  " + variableName + ".at(i).serialize(QString());";
                }
                code += "str += subValue.value().toString();";
                code += "decls += subValue.namespaceDeclarations();";
            }
            code.unindent();
            code += "}";
            code += "KDSoapValue value(valueName, str, " + namespaceString(type->nameSpace()) + ", QString::fromLatin1(\"" + type->name() + "\"));";
            code += "value.setNamespaceDeclarations(decls);";
            code += "return value;";
            serializeFunc.setBody(code);
        }
        {
            newClass.addHeaderInclude("QtCore/QStringList");
            KODE::Code code;
            code += "if (mainValue.value().toString().trimmed().isEmpty()) return;";
            code += "const KDSoapValueList list = mainValue.split();";
            code += "for (int i = 0; i < list.count(); ++i) {";
            code.indent();
            QString val = QString::fromLatin1("list.at(i)");
            if (itemTypeName == "QString")
                val = val + ".value().toString()";
            else if (mTypeMap.isBuiltinType(baseName)) {     // deserialize to int, float, bool, etc.
                val = mTypeMap.deserializeBuiltin(baseName, QName(), val, itemTypeName);
            } else {
                code += itemTypeName + " tmp;";
                code += "tmp.deserialize(" + val + ");";
                val = "tmp";
            }
            code += variableName + ".append(" + val + ");";
            code.unindent();
            code += "}";
            deserializeFunc.setBody(code);
        }
    }
    break;
    case XSD::SimpleType::TypeUnion: {
        const QString variableName = KODE::MemberVariable::memberVariableName("value");
        {
            KODE::Code code;
            code += "return KDSoapValue(valueName, " + variableName + ", " + namespaceString(type->nameSpace()) + ", QString::fromLatin1(\"" + type->name() + "\"));";
            serializeFunc.setBody(code);
        }
        {
            KODE::Code code;
            code += variableName + " = mainValue.value();" + COMMENT;
            deserializeFunc.setBody(code);
        }
        break;
    }
    }

    newClass.addFunction(serializeFunc);
    newClass.addFunction(deserializeFunc);
}

static QString escapeEnum(const QString &str)
{
    return upperlize(KODE::Style::makeIdentifier(str));
}

static QString escapeRegExp(const QString &str)
{
    QString reg = str;
    reg.replace('\\', "\\\\");
    return reg;
}

static KODE::Code createRangeCheckCode(const XSD::SimpleType *type, const QString &baseTypeName, const QString &variableName, KODE::Class &parentClass, const XSD::SimpleType &baseSimpleType)
{
    QString extendedVariableName = variableName;

    if (!baseSimpleType.isNull()) {
        if (baseSimpleType.subType() == XSD::SimpleType::SubType::TypeList) {
            extendedVariableName += ".entries()";
        } else {
            extendedVariableName += ".value()";
        }
    }

    KODE::Code code;
    code += "bool rangeOk = true;";
    code.newLine();

    // TODO range-check code for facetWhiteSpace, facetTotalDigits, facetFractionDigits
    /*
      WhiteSpaceType facetWhiteSpace() const;
      int facetTotalDigits() const;
      int facetFractionDigits() const;
    */

    if (type->facetType() & XSD::SimpleType::MININC) {
        // Don't generate if (uint >= 0) code, it does nothing, and warns
        if (!(type->facetMinimumInclusive() == 0 && (baseTypeName == "quint64" || baseTypeName.startsWith("unsigned ")))) {
            code += "rangeOk = rangeOk && (" + variableName + " >= " + QString::number(type->facetMinimumInclusive()) + ");";
        }
    }
    if (type->facetType() & XSD::SimpleType::MINEX) {
        code += "rangeOk = rangeOk && (" + variableName + " > " + QString::number(type->facetMinimumExclusive()) + ");";
    }
    if (type->facetType() & XSD::SimpleType::MAXINC) {
        code += "rangeOk = rangeOk && (" + variableName + " <= " + QString::number(type->facetMaximumInclusive()) + ");";
    }
    if (type->facetType() & XSD::SimpleType::MINEX) {
        code += "rangeOk = rangeOk && (" + variableName + " < " + QString::number(type->facetMaximumExclusive()) + ");";
    }

    if (type->facetType() & XSD::SimpleType::LENGTH) {
        code += "rangeOk = rangeOk && (" + extendedVariableName + ".length() == " + QString::number(type->facetLength()) + ");";
    }
    if (type->facetType() & XSD::SimpleType::MINLEN) {
        code += "rangeOk = rangeOk && (" + extendedVariableName + ".length() >= " + QString::number(type->facetMinimumLength()) + ");";
    }
    if (type->facetType() & XSD::SimpleType::MAXLEN) {
        code += "rangeOk = rangeOk && (" + extendedVariableName + ".length() <= " + QString::number(type->facetMaximumLength()) + ");";
    }
    if (type->facetType() & XSD::SimpleType::PATTERN) {
        if (baseTypeName == "QString") {
            code += "QRegExp exp( QString::fromLatin1(\"" + escapeRegExp(type->facetPattern()) + "\") );";
            code += "rangeOk = rangeOk && exp.exactMatch( " + variableName + " );";
        }

        parentClass.addInclude("QtCore/QRegExp");
    }

    return code;
}
