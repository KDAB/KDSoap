#include "converter.h"
#include "settings.h"
#include <libkode/style.h>

#include <QDebug>

static const char* soapEncNs = "http://schemas.xmlsoap.org/soap/encoding/";

using namespace KWSDL;

void Converter::convertComplexType( const XSD::ComplexType *type )
{
    // An empty type is still useful, in document mode: it serializes the element name
    //if ( type->isEmpty() )
    //    return;

    if ( type->nameSpace() == soapEncNs )
        return;

    const QString typeName( mTypeMap.localType( type->qualifiedName() ) );
    KODE::Class newClass( typeName );
    if (!Settings::self()->exportDeclaration().isEmpty())
      newClass.setExportDeclaration(Settings::self()->exportDeclaration());

    newClass.setUseSharedData( true, "d_ptr" /*avoid clash with possible d() method */ );

    const bool doDebug = (qgetenv("KDSOAP_TYPE_DEBUG").toInt());
    if (doDebug)
        qDebug() << "Generating complex type" << typeName;

    // subclass handling
    if ( !type->baseTypeName().isEmpty() ) { // this class extends something
        /**
         * A class can't subclass basic type (e.g. int, unsigned char), so we
         * add setValue() and value() methods to access the base type.
         *
         * In fact, let's do the same with string
         */
        if ( type->baseTypeName().localName() == "Array" ) {
            // this is handled in the attribute section
        } else {
            const QName baseName = type->baseTypeName();
            const QString typeName = mTypeMap.localType( baseName );
            const QString inputTypeName = mTypeMap.localInputType( baseName, QName() );

            // include header
            newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( baseName ) );
            newClass.addHeaderIncludes( mTypeMap.headerIncludes( baseName ) );

            // member variables
            KODE::MemberVariable variable( "value", typeName );
            addVariableInitializer( variable );
            newClass.addMemberVariable( variable );

            const QString variableName = "d_ptr->" + variable.name();

            // setter method
            KODE::Function setter( "setValue", "void" );
            setter.addArgument( inputTypeName + " value" );
            setter.setBody( variableName + " = value;" );

            // getter method
            KODE::Function getter( "value", typeName );
            getter.setBody( "return " + variableName + ';' );
            getter.setConst( true );

            // convenience constructor
            KODE::Function conctor( upperlize( newClass.name() ) );
            conctor.addArgument( inputTypeName + " value" );
            conctor.setBody( variableName + " = value;" );

            // type operator
            KODE::Function op( "operator const " + typeName );
            op.setBody( "return " + variableName + ';' );
            op.setConst( true );

            newClass.addFunction( conctor );
            newClass.addFunction( op );
            newClass.addFunction( setter );
            newClass.addFunction( getter );
        }
    }

    if ( !type->documentation().isEmpty() )
        newClass.setDocs( type->documentation().simplified() );

    // elements
    const XSD::Element::List elements = type->elements();
    Q_FOREACH( const XSD::Element &elemIt, elements ) {

        //qDebug() << elemIt.name() << elemIt.qualifiedName() << elemIt.type();
        if (elemIt.type().isEmpty()) {
            qDebug() << "ERROR: Element with no type:" << elemIt.name() << "(skipping)";
            Q_ASSERT(false);
            continue;
        }
        QString typeName = mTypeMap.localType( elemIt.type() );
        Q_ASSERT(!typeName.isEmpty());

        if (typeName != "void") // void means empty element, probably just here for later extensions (testcase: SetPasswordResult in salesforce)
        {
            QString inputTypeName = mTypeMap.localInputType( elemIt.type(), QName() );

            if ( elemIt.maxOccurs() > 1 ) {
                typeName = listTypeFor(typeName, newClass);
                inputTypeName = "const " + typeName + "&";
            }
            if ( type->isArray() ) {
                const QString arrayTypeName = mTypeMap.localType( type->arrayType() );
                Q_ASSERT(!arrayTypeName.isEmpty());
                //qDebug() << "array of" << attribute.arrayType() << "->" << arrayTypeName;
                typeName = listTypeFor(arrayTypeName, newClass);
                newClass.addInclude(QString(), arrayTypeName); // add forward declaration
                newClass.addHeaderIncludes( QStringList() << "QList" );
                inputTypeName = "const " + typeName + '&';
            }

            // member variables
            KODE::MemberVariable variable( elemIt.name(), typeName );
            addVariableInitializer( variable );
            newClass.addMemberVariable( variable );

            const QString variableName = "d_ptr->" + variable.name();

            const QString upperName = upperlize( elemIt.name() );
            const QString lowerName = lowerlize( elemIt.name() );

            // setter method
            KODE::Function setter( "set" + upperName, "void" );
            setter.addArgument( inputTypeName + ' ' + mNameMapper.escape( lowerName ) );
            setter.setBody( variableName + " = " + mNameMapper.escape( lowerName ) + ';' );

            // getter method
            KODE::Function getter( mNameMapper.escape( lowerName ), typeName );
            getter.setBody( "return " + variableName + ';' );
            getter.setConst( true );

            newClass.addFunction( setter );
            newClass.addFunction( getter );
        }

        // include header
        newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( elemIt.type() ) );
        newClass.addHeaderIncludes( mTypeMap.headerIncludes( elemIt.type() ) );
        if ( elemIt.maxOccurs() > 1 )
            newClass.addHeaderIncludes( QStringList( "QList" ) );
    }

    // attributes
    XSD::Attribute::List attributes = type->attributes();
    Q_FOREACH(const XSD::Attribute& attribute, attributes) {
        QString typeName, inputTypeName;

        typeName = mTypeMap.localType( attribute.type() );
        if (typeName.isEmpty()) {
            qDebug() << "ERROR: attribute with unknown type:" << attribute.name() << attribute.type() << "in" << typeName;
        }
        inputTypeName = mTypeMap.localInputType( attribute.type(), QName() );
        //qDebug() << "Attribute" << attribute.name();

        // member variables
        KODE::MemberVariable variable( attribute.name(), typeName );
        addVariableInitializer( variable );
        newClass.addMemberVariable( variable );
        const QString variableName = "d_ptr->" + variable.name();

        QString upperName = upperlize( attribute.name() );
        QString lowerName = lowerlize( attribute.name() );

        // setter method
        KODE::Function setter( "set" + upperName, "void" );
        setter.addArgument( inputTypeName + ' ' + mNameMapper.escape( lowerName ) );
        setter.setBody( variableName + " = " + mNameMapper.escape( lowerName ) + ';' );

        // getter method
        KODE::Function getter( mNameMapper.escape( lowerName ), typeName );
        getter.setBody( "return " + variableName + ';' );
        getter.setConst( true );

        newClass.addFunction( setter );
        newClass.addFunction( getter );

        // include header
        newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( attribute.type() ) );
        newClass.addHeaderIncludes( mTypeMap.headerIncludes( attribute.type() ) );
    }

    createComplexTypeSerializer( newClass, type );

    KODE::Function ctor( upperlize( newClass.name() ) );
    newClass.addFunction( ctor );

    KODE::Function dtor( '~' + upperlize( newClass.name() ) );
    newClass.addFunction( dtor );

    mClasses.append( newClass );
}

static QString namespaceString(const QString& ns)
{
    if (ns == QLatin1String("http://www.w3.org/1999/XMLSchema"))
        return "KDSoapNamespaceManager::xmlSchema1999()";
    if (ns == QLatin1String("http://www.w3.org/2001/XMLSchema"))
        return "KDSoapNamespaceManager::xmlSchema2001()";
    //qDebug() << "got namespace" << ns;
    // TODO register into KDSoapNamespaceManager? This means generating code in the clientinterface ctor...
    return "QString::fromLatin1(\"" + ns + "\")";
}

// Helper method for the generation of the serialize() method, also used in addMessageArgument.
KODE::Code Converter::serializeElementArg( const QName& type, const QName& elementType, const QString& name, const QString& localVariableName, const QByteArray& varName, bool append )
{
    const QString varAndMethod = varName + (append ? ".append" : " = ");
    KODE::Code block;
    // for debugging, add this:
    //block += "// type: " + type.qname() + " element:" + elementType.qname();
    //qDebug() << "appendElementArg: type=" << type << "isBuiltin=" << mTypeMap.isBuiltinType(type);
    if ( mTypeMap.isTypeAny( type ) ) {
        block += "if (!" + localVariableName + ".isNull()) {";
        block.indent();
        block += varAndMethod + '(' + localVariableName + ");" COMMENT;
        block.unindent();
        block += "}";
    } else {
        const QString nameArg = "QString::fromLatin1(\"" + name + "\")";
        const QName actualType = type.isEmpty() ? elementType : type;
        const QString typeArgs = namespaceString(actualType.nameSpace()) + ", QString::fromLatin1(\"" + actualType.localName() + "\")";
        if ( mTypeMap.isBuiltinType( type, elementType ) ) {
            const QString qtTypeName = mTypeMap.localType( type, elementType );
            const QString value = mTypeMap.serializeBuiltin( type, elementType, localVariableName, qtTypeName );

            block += varAndMethod + "(KDSoapValue(" + nameArg + ", " + value + ", " + typeArgs + "));" COMMENT;
        } else if ( mTypeMap.isComplexType( type, elementType ) ) {
            block += varAndMethod + '(' + localVariableName + ".serialize(" + nameArg + "));" COMMENT;
        } else {
            block += varAndMethod + "(KDSoapValue(" + nameArg + ", " + localVariableName + ".serialize(), " + typeArgs + "));" COMMENT;
        }
    }
    return block;
}

// Helper method for the generation of the deserialize() method
static KODE::Code demarshalNameTest( const QName& type, const QString& tagName, bool *first )
{
    KODE::Code demarshalCode;
    if ( type.nameSpace() == XMLSchemaURI && (type.localName() == "any") ) {
        demarshalCode += QString::fromLatin1(*first? "" : "else ") + "{" COMMENT;
    } else {
        demarshalCode += QString::fromLatin1(*first? "" : "else ") + "if (name == QLatin1String(\"" + tagName + "\")) {" COMMENT;
    }
    *first = false;
    return demarshalCode;
}

// Helper method for the generation of the deserialize() method, also used by convertClientCall
KODE::Code Converter::demarshalVar( const QName& type, const QName& elementType, const QString& variableName, const QString& qtTypeName, const QString& soapValueVarName ) const
{
    KODE::Code code;
    if ( mTypeMap.isTypeAny( type ) ) {
        code += variableName + " = " + soapValueVarName + ";" COMMENT;
    } else if ( mTypeMap.isBuiltinType( type, elementType ) ) {
        code += variableName + " = " + mTypeMap.deserializeBuiltin(type, elementType, soapValueVarName + ".value()", qtTypeName) + ";" COMMENT;
    } else if ( mTypeMap.isComplexType( type, elementType ) ) {
        code += variableName + ".deserialize(" + soapValueVarName + ");" COMMENT;
    } else {
        code += variableName + ".deserialize(" + soapValueVarName + ".value());" COMMENT;
    }
    return code;
}

KODE::Code Converter::demarshalArrayVar( const QName& type, const QString& variableName, const QString& typeName ) const
{
    KODE::Code code;
    if ( mTypeMap.isTypeAny( type ) ) { // KDSoapValue doesn't support temp vars [still true?]. This special-casing is ugly though.
        code += variableName + ".append(val);";
    } else {
        // we need a temp var in case of deserialize()
        // [TODO: we could merge demarshalVar into this code, to avoid the temp var in other cases]
        QString tempVar;
        if (variableName.startsWith("d_ptr->"))
            tempVar = variableName.mid(7) + "Temp";
        else
            tempVar = variableName + "Temp";
        code += typeName + " " + tempVar + ";";
        code.addBlock( demarshalVar( type, QName(), tempVar, typeName ) );
        code += variableName + ".append(" + tempVar + ");";
    }
    return code;
}

void Converter::createComplexTypeSerializer( KODE::Class& newClass, const XSD::ComplexType *type )
{
    newClass.addInclude("KDSoapNamespaceManager.h");

    KODE::Function serializeFunc( "serialize", "KDSoapValue" );
    serializeFunc.addArgument( "const QString& valueName" );
    serializeFunc.setConst( true );

    KODE::Function deserializeFunc( "deserialize", "void" );
    deserializeFunc.addArgument( "const KDSoapValue& mainValue" );

    KODE::Code marshalCode, demarshalCode;

    const QString typeArgs = namespaceString(type->nameSpace()) + ", QString::fromLatin1(\"" + type->name() + "\")";

    if ( type->baseTypeName() != XmlAnyType && !type->baseTypeName().isEmpty() && !type->isArray() ) {

        const QName baseName = type->baseTypeName();
        const QString typeName = mTypeMap.localType( baseName );
        KODE::MemberVariable variable( "value", typeName );
        const QString variableName = "d_ptr->" + variable.name();

        if ( mTypeMap.isComplexType( baseName ) ) {
            marshalCode += "KDSoapValue mainValue = " + variableName + ".serialize(valueName);" + COMMENT;
            marshalCode += "mainValue.setType(" + typeArgs + ");";
        } else {
            QString value;
            if ( mTypeMap.isBuiltinType( baseName ) )
                value = mTypeMap.serializeBuiltin( baseName, QName(), variableName, typeName );
            else
                value += variableName + ".serialize()";
            marshalCode += "KDSoapValue mainValue(valueName, " + value + ", " + typeArgs + ");" + COMMENT;
        }

        demarshalCode += demarshalVar( baseName, QName(), variableName, typeName, "mainValue" );
    } else {
        marshalCode += "KDSoapValue mainValue(valueName, QVariant(), " + typeArgs + ");" + COMMENT;
    }

    // elements
    XSD::Element::List elements = type->elements();

    // remove "void" elements (testcase: "result" in salesforce-partner's setPasswordResponse)
    QMutableListIterator<XSD::Element> itElem(elements);
    while (itElem.hasNext()) {
        const XSD::Element& elem = itElem.next();
        const QString typeName = mTypeMap.localType( elem.type() );
        if (typeName == "void")
            itElem.remove();
    }
    const XSD::Attribute::List attributes = type->attributes();
    if ( !elements.isEmpty() || !attributes.isEmpty() ) {
        demarshalCode += "const KDSoapValueList& args = mainValue.childValues();" COMMENT;
    }

    if ( !elements.isEmpty() ) {
        marshalCode += "KDSoapValueList& args = mainValue.childValues();" COMMENT;
        demarshalCode += "for (int argNr = 0; argNr < args.count(); ++argNr) {";
        demarshalCode.indent();
        demarshalCode += "const KDSoapValue& val = args.at(argNr);";
        demarshalCode += "const QString name = val.name();";
    } else {
        // The Q_UNUSED is not necessarily true in case of attributes, but who cares.
        demarshalCode += "Q_UNUSED(mainValue);" COMMENT;
    }

    if ( type->isArray() ) {
        if (elements.count() != 1) {
            qDebug() << "array" << type->name() << "has" << elements.count() << "elements!";
        }
        Q_ASSERT(elements.count() == 1);
        const XSD::Element elem = elements.first();
        //const QString typeName = mTypeMap.localType( elem.type() );
        //Q_ASSERT(!typeName.isEmpty());
        KODE::MemberVariable variable( elem.name(), "whatever" ); // was already added; this is just for the naming
        const QString variableName = "d_ptr->" + variable.name(); // always d_ptr->mEntries, actually
        const QName arrayType = type->arrayType();
        const QString typeName = mTypeMap.localType( arrayType );

        marshalCode += "args.setArrayType(QString::fromLatin1(\"" + arrayType.nameSpace() + "\"), QString::fromLatin1(\"" + arrayType.localName() + "\"));";
        marshalCode += "for (int i = 0; i < " + variableName + ".count(); ++i) {";
        marshalCode.indent();
        marshalCode.addBlock( serializeElementArg( arrayType, QName(), "item", variableName + ".at(i)", "args", true ) );
        marshalCode.unindent();
        marshalCode += '}';

        demarshalCode.addBlock( demarshalArrayVar( arrayType, variableName, typeName ) );
    } else {
        bool first = true;
        Q_FOREACH( const XSD::Element& elem, elements ) {

            const QString elemName = elem.name();
            const QString typeName = mTypeMap.localType( elem.type() );
            Q_ASSERT(!typeName.isEmpty());
            Q_ASSERT(typeName != "void"); // removed earlier in this file

            KODE::MemberVariable variable( elemName, typeName ); // was already added; this is just for the naming
            const QString variableName = "d_ptr->" + variable.name();

            demarshalCode.addBlock( demarshalNameTest( elem.type(), elemName, &first ) );
            demarshalCode.indent();

            if ( elem.maxOccurs() > 1 ) {
                //const QString typePrefix = mNSManager.prefix( elem.type().nameSpace() );

                marshalCode += "for (int i = 0; i < " + variableName + ".count(); ++i) {";
                marshalCode.indent();
                marshalCode.addBlock( serializeElementArg( elem.type(), QName(), elem.name(), variableName + ".at(i)", "args", true ) );
                marshalCode.unindent();
                marshalCode += '}';

                demarshalCode.addBlock( demarshalArrayVar( elem.type(), variableName, typeName ) );
            } else {
                marshalCode.addBlock( serializeElementArg( elem.type(), QName(), elem.name(), variableName, "args", true ) );
                demarshalCode.addBlock( demarshalVar( elem.type(), QName(), variableName, typeName ) );
            }

            demarshalCode.unindent();
            demarshalCode += "}";
        } // end: for each element
    }

    if ( !elements.isEmpty() ) {
        demarshalCode.unindent();
        demarshalCode += "}";
    }

    if ( !attributes.isEmpty() ) {

        marshalCode += "KDSoapValueList attribs;";

        demarshalCode += "const QList<KDSoapValue> attribs = args.attributes();";
        demarshalCode += "for (int attrNr = 0; attrNr < attribs.count(); ++attrNr) {";
        demarshalCode.indent();
        demarshalCode += "const KDSoapValue& val = attribs.at(attrNr);";
        demarshalCode += "const QString name = val.name();";

        bool first = true;
        Q_FOREACH( const XSD::Attribute& attribute, attributes ) {
            const QString attrName = attribute.name();
            KODE::MemberVariable variable( attrName, "doesnotmatter" ); // was already added; this is just for the naming
            const QString variableName = "d_ptr->" + variable.name();

            demarshalCode.addBlock( demarshalNameTest( attribute.type(), attrName, &first ) );
            demarshalCode.indent();

            marshalCode.addBlock( serializeElementArg( attribute.type(), QName(), attribute.name(), variableName, "attribs", true ) );

            const QString typeName = mTypeMap.localType( attribute.type() );
            Q_ASSERT(!typeName.isEmpty());
            demarshalCode.addBlock( demarshalVar( attribute.type(), QName(), variableName, typeName ) );

            demarshalCode.unindent();
            demarshalCode += "}";
        }
        marshalCode += "mainValue.childValues().attributes() += attribs;" COMMENT;

        demarshalCode.unindent();
        demarshalCode += "}";
    }

    marshalCode += "return mainValue;";

    serializeFunc.setBody( marshalCode );
    newClass.addFunction( serializeFunc );

    deserializeFunc.setBody( demarshalCode );
    newClass.addFunction( deserializeFunc );
}
