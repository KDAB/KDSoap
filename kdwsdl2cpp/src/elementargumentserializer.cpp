#include "elementargumentserializer.h"
#include "converter.h" // upperlize(), COMMENT
#include <libkode/style.h>

using namespace KWSDL;

ElementArgumentSerializer::ElementArgumentSerializer(const TypeMap &typeMap, const QName &type, const QName &elementType, const QString &localVarName)
    : mTypeMap(typeMap),
      mType(type),
      mElementType(elementType),
      mLocalVarName(localVarName),
      mAppend(false),
      mIsQualified(false),
      mNillable(false),
      mOmitIfEmpty(false),
      mUsePointer(false)
{

}

void ElementArgumentSerializer::setLocalVariableName(const QString &localVarName)
{
    mLocalVarName = localVarName;
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

void ElementArgumentSerializer::setOmitIfEmpty(bool omit)
{
    mOmitIfEmpty = omit;
}

void ElementArgumentSerializer::setNillable(bool nillable)
{
    mNillable = nillable;
}

void ElementArgumentSerializer::setIsQualified(bool qualified)
{
    mIsQualified = qualified;
}

void ElementArgumentSerializer::setUsePointer(bool usePointer)
{
    mUsePointer = usePointer;
}

KODE::Code ElementArgumentSerializer::generate() const
{
    Q_ASSERT(!mLocalVarName.isEmpty());
    Q_ASSERT(!mOutputVarName.isEmpty());
    const QString varAndMethodBefore = mOutputVarName + (mAppend ? QLatin1String(".append(") : QLatin1String(" = "));
    const QString varAndMethodAfter = mAppend ? QString::fromLatin1(")") : QString();

    KODE::Code block;
    // for debugging, add this:
    //block += "// type: " + type.qname() + " element:" + elementType.qname();

    //if ( name.localName() == "..." )
    //    qDebug() << "appendElementArg:" << name << "type=" << type << "isBuiltin=" << mTypeMap.isBuiltinType(type) << "isQualified=" << isQualified;
    if (mTypeMap.isTypeAny(mType)) {
        block += QLatin1String("if (!") + mLocalVarName + QLatin1String(".isNull()) {");
        block.indent();
        block += varAndMethodBefore + mLocalVarName + varAndMethodAfter + QLatin1String(";") + COMMENT;
        block.unindent();
        block += "}";
    } else {
        const QName actualType = mType.isEmpty() ? mElementType : mType;
        const QString typeArgs = namespaceString(actualType.nameSpace()) + QLatin1String(", QString::fromLatin1(\"") + actualType.localName() + QLatin1String("\")");
        const bool isComplex = mTypeMap.isComplexType(mType, mElementType);
        const bool isPolymorphic = mTypeMap.isPolymorphic(mType, mElementType);

        if (mAppend && mOmitIfEmpty) {
            if (mUsePointer) {
                block += "if (" + mLocalVarName + ") {";
            } else {
                block += "if (!" + mLocalVarName + "_nil) {";
            }
            block.indent();
        }

        if (isComplex) {
            const QString op = (isPolymorphic || mUsePointer) ? "->" : ".";
            block += QLatin1String("KDSoapValue ") + mValueVarName + QLatin1Char('(') + mLocalVarName + op + QLatin1String("serialize(") + mNameArg + QLatin1String("));") + COMMENT;
        } else {
            if (mTypeMap.isBuiltinType(mType, mElementType)) {
                const QString qtTypeName = mTypeMap.localType(mType, mElementType);
                const QString value = mTypeMap.serializeBuiltin(mType, mElementType, mLocalVarName, qtTypeName);

                block += QLatin1String("KDSoapValue ") + mValueVarName + QLatin1String("(") + mNameArg + QLatin1String(", ") + value + QLatin1String(", ") + typeArgs + QLatin1String(");") + COMMENT;
            } else {
                block += QLatin1String("KDSoapValue ") + mValueVarName + QLatin1String("(") + mNameArg + QLatin1String(", ") + mLocalVarName + QLatin1String(".serialize(), ") + typeArgs + QLatin1String(");") + COMMENT;
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
        if (mAppend && mOmitIfEmpty) {   // omit empty children (testcase: MSExchange, no <ParentFolderIds/>)
            block += "if (!" + mValueVarName + ".isNil())";
        }
        block += varAndMethodBefore + mValueVarName + varAndMethodAfter + QLatin1String(";") + COMMENT;

        if (mAppend && mOmitIfEmpty) {
            block.unindent();
            block += "}";
        }
    }
    return block;
}

