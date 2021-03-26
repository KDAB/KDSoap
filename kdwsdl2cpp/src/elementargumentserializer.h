#ifndef ELEMENTARGUMENTSERIALIZER_H
#define ELEMENTARGUMENTSERIALIZER_H

#include "typemap.h"
#include <code_generation/code.h>

/**
 * The ElementArgumentSerializer class is used to generate the code that serializes one XML element
 * (i.e. one KDSoapValue)
 *
 * It is used in the generation of the serialize() method, and is also used in addMessageArgument.
 */
class ElementArgumentSerializer
{
public:
    /**
     * Constructor
     * @param type the name of the XSD type. Exclusive with elementType.
     * @param elementType the name of the XSD element type. Exclusive with type.
     * @param localVarName the name of the variable containing the type to send, in the generated C++ code
     * If it's not a builtin type, .serialize() will be called on this variable.
     * @param nilLocalVarName name of the _nil variable, if any
     */
    ElementArgumentSerializer(const KWSDL::TypeMap &typeMap, const QName &type, const QName &elementType, const QString &localVarName, const QString &nilLocalVarName);

    /**
     * Modifies the localVarName set in the constructor.
     * @param localVarName the name of the variable containing the type to send, in the generated C++ code
     * If it's not a builtin type, .serialize() will be called on this variable.
     */
    void setLocalVariableName(const QString &localVarName);

    /**
     * @brief sets the element name, i.e. the literal string that will become the XML element name.
     * @param name element name including namespace. Unused if type is any.
     */
    void setElementName(const QName &name);

    /**
     * @brief sets the element name to be the result of a method call.
     * This is exclusive with setElementName, and allows to implement substitution groups.
     */
    void setDynamicElementName(const QString &codeLocalName, const QString &codeNamespace, const QName &baseName);

    /**
     * @brief sets the name of the output variable, and whether to set that variable ("var = ...")
     * to append to it ("var.append(...")).
     *
     * @param outputVarName the name of the output variable
     * @param append whether to append (true) or set (false)
     */
    void setOutputVariable(const QString &outputVarName, bool append);

    /**
     * @brief sets whether the element is optional.
     * For serialization, this means it will be omitted if it's empty.
     * For deserialization, it means setting _nil to false if present.
     * The default is false.
     */
    void setOptional(bool optional);

    /**
     * @brief sets whether the element can be nil (xsi:nil)
     * @param nillable use xsi:nil if true
     * The default is false.
     */
    void setNillable(bool nillable);

    /**
     * @brief sets whether the element is qualified, see KDSoapValue::setQualified(bool)
     * @param qualified
     * The default is false.
     */
    void setIsQualified(bool qualified);

    /**
     * @brief sets whether to use -> instead of . because the variable is a pointer
     * @param qualified
     * The default is false.
     */
    void setUsePointer(bool usePointer);

    /**
     * The main method: generate the serialization code.
     * @return the generated code
     */
    KODE::Code generateSerializationCode() const;

    /**
     * Generate code to deserialize an entire array
     * @return the generated code
     */
    KODE::Code demarshalArray(const QString &soapValueVarName) const;

    /**
     * Generate code to deserialize the variable
     * @param the name of the variable containing the KDSoapValue to read from
     * @return the generated code
     */
    KODE::Code demarshalVariable(const QString &soapValueVarName) const;

    static QString pointerStorageType(const QString &typeName);

private:
    // Low-level helper for demarshalVariable, doesn't handle the polymorphic case (so it can be called for lists of polymorphics)
    KODE::Code demarshalVarHelper(const QString &soapValueVarName) const;

    const KWSDL::TypeMap &mTypeMap;
    QName mType;
    QName mElementType;
    QString mNameArg;
    QString mNameNamespace;
    QString mLocalVarName;
    QString mNilLocalVarName;
    QString mOutputVarName;
    QString mValueVarName;
    bool mAppend;
    bool mIsQualified;
    bool mNillable;
    bool mOptional;
    bool mUsePointer;
};

#endif // ELEMENTARGUMENTSERIALIZER_H
