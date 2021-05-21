#include "elementargumentserializer.h"
#include "converter.h" // upperlize(), COMMENT
#include <code_generation/style.h>

using namespace KWSDL;

ElementArgumentSerializer::ElementArgumentSerializer(const TypeMap &typeMap, const QName &type, const QName &elementType, const QString &localVarName,
                                                     const QString &nilLocalVarName)
    : mTypeMap(typeMap)
    , mType(type)
    , mElementType(elementType)
    , mLocalVarName(localVarName)
    , mNilLocalVarName(nilLocalVarName)
    , mAppend(false)
    , mIsQualified(false)
    , mNillable(false)
    , mOptional(false)
    , mUsePointer(false)
{
}

void ElementArgumentSerializer::setLocalVariableName(const QString &localVarName)
{
    mLocalVarName = localVarName;
    Q_ASSERT(!mNillable); // if nillable, we're missing mNilLocalVarName here
}

void ElementArgumentSerializer::setElementName(const QName &name)
{
    mNameArg = QLatin1String("QString::fromLatin1(\"") + name.localName() + QLatin1String("\")");
    mNameNamespace = namespaceString(name.nameSpace());
    mValueVarName = QLatin1String("_value") + upperlize(KODE::Style::makeIdentifier(name.localName()));
}

void ElementArgumentSerializer::setDynamicElementName(const QString &codeLocalName, const QString &codeNamespace, const QName &baseName)
{
    mNameArg = codeLocalName;
    mNameNamespace = codeNamespace;
    mValueVarName = QLatin1String("_value") + upperlize(KODE::Style::makeIdentifier(baseName.localName()));
}

void ElementArgumentSerializer::setOutputVariable(const QString &outputVarName, bool append)
{
    mOutputVarName = outputVarName;
    mAppend = append;
}

void ElementArgumentSerializer::setNillable(bool nillable)
{
    mNillable = nillable;
}

void ElementArgumentSerializer::setOptional(bool optional)
{
    mOptional = optional;
}

void ElementArgumentSerializer::setIsQualified(bool qualified)
{
    mIsQualified = qualified;
}

void ElementArgumentSerializer::setUsePointer(bool usePointer)
{
    mUsePointer = usePointer;
}

KODE::Code ElementArgumentSerializer::generateSerializationCode() const
{
    Q_ASSERT(!mLocalVarName.isEmpty());
    Q_ASSERT(!mOutputVarName.isEmpty());
    const QString varAndMethodBefore = mOutputVarName + (mAppend ? QLatin1String(".append(") : QLatin1String(" = "));
    const QString varAndMethodAfter = mAppend ? QString::fromLatin1(")") : QString();

    KODE::Code block;
    // for debugging, add this:
    // block += "// type: " + type.qname() + " element:" + elementType.qname();

    // if ( name.localName() == "..." )
    //    qDebug() << "appendElementArg:" << name << "type=" << type << "isBuiltin=" << mTypeMap.isBuiltinType(type) << "isQualified=" << isQualified;
    if (mTypeMap.isTypeAny(mType)) {
        block += QLatin1String("if (!") + mLocalVarName + QLatin1String(".isNull()) {");
        block.indent();
        block += varAndMethodBefore + mLocalVarName + varAndMethodAfter + QLatin1String(";") + COMMENT;
        block.unindent();
        block += "}";
    } else {
        const QName actualType = mType.isEmpty() ? mElementType : mType;
        // UNUSED const QString typeArgs = namespaceString(actualType.nameSpace()) + QLatin1String(", QString::fromLatin1(\"") +
        // actualType.localName() + QLatin1String("\")");
        const bool isComplex = mTypeMap.isComplexType(mType, mElementType);
        const bool isPolymorphic = mTypeMap.isPolymorphic(mType, mElementType);

        if (mAppend && mOptional) {
            if (mUsePointer) {
                block += "if (" + mLocalVarName + ") {";
            } else {
                block += "if (!" + mNilLocalVarName + ") {" + COMMENT;
            }
            block.indent();
        }

        if (isComplex) {
            const QString op = (isPolymorphic || mUsePointer) ? "->" : ".";
            block += QLatin1String("KDSoapValue ") + mValueVarName + QLatin1Char('(') + mLocalVarName + op + QLatin1String("serialize(") + mNameArg
                + QLatin1String("));") + COMMENT;
        } else {
            if (mTypeMap.isBuiltinType(mType, mElementType)) {
                const QString value =
                    mTypeMap.serializeBuiltin(mType, mElementType, mLocalVarName, mNameArg, actualType.nameSpace(), actualType.localName());
                block += QLatin1String("KDSoapValue ") + mValueVarName + QLatin1String(" = ") + value + QLatin1String(";") + COMMENT;
            } else {
                block += QLatin1String("KDSoapValue ") + mValueVarName + QLatin1String(" = ") + mLocalVarName + QLatin1String(".serialize(")
                    + mNameArg + QLatin1String(");") + COMMENT;
            }
        }
        if (!mNameNamespace.isEmpty()) {
            block += mValueVarName + QLatin1String(".setNamespaceUri(") + mNameNamespace + QLatin1String(");");
        }
        if (mIsQualified) {
            block += mValueVarName + QLatin1String(".setQualified(true);");
        }
        if (mNillable) {
            block += mValueVarName + QLatin1String(".setNillable(true);");
        }
        block += varAndMethodBefore + mValueVarName + varAndMethodAfter + QLatin1String(";") + COMMENT;

        if (mAppend && mOptional) {
            block.unindent();
            block += "}";
        }
    }
    return block;
}

QString ElementArgumentSerializer::pointerStorageType(const QString &typeName)
{
    if (typeName == "QString" || typeName == "bool") {
        qWarning() << "Should not happen: polymorphic" << typeName;
        Q_ASSERT(0);
    }

    return "QSharedPointer<" + typeName + '>';
}

KODE::Code ElementArgumentSerializer::demarshalArray(const QString &soapValueVarName) const
{
    KODE::Code code;
    const QString qtTypeName = mTypeMap.localType(mType, mElementType);

    if (mTypeMap.isTypeAny(mType)) {
        code += mLocalVarName + QLatin1String(".append(") + soapValueVarName + QLatin1String(");");
    } else if (mTypeMap.isBuiltinType(mType, mElementType)) {
        code += mLocalVarName + QLatin1String(".append(") + mTypeMap.deserializeBuiltin(mType, mElementType, soapValueVarName, qtTypeName)
            + QLatin1String(");") + COMMENT;
    } else {
        // we need a temp var because of deserialize()
        QString tempVar;
        if (mLocalVarName.startsWith(QLatin1String("d_ptr->"))) {
            tempVar = mLocalVarName.mid(7) + QLatin1String("Temp");
        } else {
            tempVar = mLocalVarName + QLatin1String("Temp");
        }
        code += qtTypeName + QLatin1String(" ") + tempVar + QLatin1String(";") + COMMENT;
        const ElementArgumentSerializer tempDeserializer(mTypeMap, mType, mElementType, tempVar, QString());
        code.addBlock(tempDeserializer.demarshalVarHelper(soapValueVarName));
        QString toAppend = tempVar;
        const bool isPolymorphic = mTypeMap.isPolymorphic(mType);
        if (isPolymorphic) {
            const QString storageType = pointerStorageType(qtTypeName);
            toAppend = storageType + "(new " + qtTypeName + "(" + tempVar + "))";
        }
        code += mLocalVarName + QLatin1String(".append(") + toAppend + QLatin1String(");") + COMMENT;
    }
    if (mOptional) {
        code += mNilLocalVarName + QLatin1String(" = false;") + COMMENT;
    }
    return code;
}

KODE::Code ElementArgumentSerializer::demarshalVariable(const QString &soapValueVarName) const
{
    const bool isPolymorphic = mTypeMap.isPolymorphic(mType, mElementType);
    if (mUsePointer || isPolymorphic) {
        const QString qtTypeName = mTypeMap.localType(mType, mElementType);
        const QString storageType = pointerStorageType(qtTypeName);
        KODE::Code code;
        code += mLocalVarName + "_as_kdsoap_value = " + soapValueVarName + ";" + COMMENT;
        code += "if (!" + mLocalVarName + ")";
        code.indent();
        code += mLocalVarName + " = " + storageType + "(new " + qtTypeName + ");" + COMMENT;
        code.unindent();
        code += mLocalVarName + QLatin1String("->deserialize(") + soapValueVarName + QLatin1String(");") + COMMENT;
        return code;
    } else {
        return demarshalVarHelper(soapValueVarName);
    }
}

KODE::Code ElementArgumentSerializer::demarshalVarHelper(const QString &soapValueVarName) const
{
    KODE::Code code;
    if (mTypeMap.isTypeAny(mType)) {
        code += mLocalVarName + QLatin1String(" = ") + soapValueVarName + QLatin1String(";") + COMMENT;
    } else if (mTypeMap.isBuiltinType(mType, mElementType)) {
        const QString qtTypeName = mTypeMap.localType(mType, mElementType);
        code += mLocalVarName + QLatin1String(" = ") + mTypeMap.deserializeBuiltin(mType, mElementType, soapValueVarName, qtTypeName)
            + QLatin1String(";") + COMMENT;
    } else if (mTypeMap.isComplexType(mType, mElementType)) {
        code += mLocalVarName + QLatin1String(".deserialize(") + soapValueVarName + QLatin1String(");") + COMMENT;
    } else {
        code += mLocalVarName + QLatin1String(".deserialize(") + soapValueVarName + QLatin1String(");") + COMMENT;
    }
    if (mOptional) {
        code += mNilLocalVarName + QLatin1String(" = false;") + COMMENT;
    }
    return code;
}
