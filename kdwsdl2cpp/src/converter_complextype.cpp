#include "converter.h"
#include "settings.h"
#include <libkode/style.h>

#include <QDebug>

using namespace KWSDL;

void Converter::convertComplexType( const XSD::ComplexType *type )
{
    // An empty type is still useful, in document mode: it serializes the element name
    //if ( type->isEmpty() )
    //    return;

    // Skip the Array types we added in Parser::init...
    if ( NSManager::soapEncNamespaces().contains(type->nameSpace()) )
        return;

    const QString className( mTypeMap.localType( type->qualifiedName() ) );
    KODE::Class newClass;
    newClass.setNamespaceAndName( className );
    if (!Settings::self()->exportDeclaration().isEmpty())
      newClass.setExportDeclaration(Settings::self()->exportDeclaration());

    newClass.setUseSharedData( true, QLatin1String("d_ptr") /*avoid clash with possible d() method */ );

    const bool doDebug = (qgetenv("KDSOAP_TYPE_DEBUG").toInt());
    if (doDebug)
        qDebug() << "Generating complex type" << className;

    // subclass handling
    if ( !type->baseTypeName().isEmpty() ) { // this class extends something
        /**
         * A class can't subclass basic type (e.g. int, unsigned char), so we
         * add setValue() and value() methods to access the base type.
         *
         * In fact, let's do the same with string
         */
        if ( type->baseTypeName().localName() == QLatin1String("Array") ) {
            // this is handled in the attribute section
        } else {
            const QName baseName = type->baseTypeName();
            const QString typeName = mTypeMap.localType( baseName );
            const QString inputTypeName = mTypeMap.localInputType( baseName, QName() );

            // include header
            newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( baseName ) );
            newClass.addHeaderIncludes( mTypeMap.headerIncludes( baseName ) );

            // member variables
            KODE::MemberVariable variable( QLatin1String("value"), typeName );
            addVariableInitializer( variable );
            newClass.addMemberVariable( variable );

            const QString variableName = QLatin1String("d_ptr->") + variable.name();

            // setter method
            KODE::Function setter( QLatin1String("setValue"), QLatin1String("void") );
            setter.addArgument( inputTypeName + QLatin1String(" value") );
            setter.setBody( variableName + QLatin1String(" = value;") );

            // getter method
            KODE::Function getter( QLatin1String("value"), typeName );
            getter.setBody( QLatin1String("return ") + variableName + QLatin1Char(';') );
            getter.setConst( true );

            // convenience constructor
            KODE::Function conctor( upperlize( newClass.name() ) );
            conctor.addArgument( inputTypeName + QLatin1String(" value") );
            conctor.setBody( variableName + QLatin1String(" = value;") );

            // type operator
            KODE::Function op( QLatin1String("operator ") + typeName );
            op.setBody( QLatin1String("return ") + variableName + QLatin1Char(';') );
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

        if (typeName != QLatin1String("void")) // void means empty element, probably just here for later extensions (testcase: SetPasswordResult in salesforce)
        {
            QString inputTypeName = mTypeMap.localInputType( elemIt.type(), QName() );

            if ( elemIt.maxOccurs() > 1 ) {
                typeName = listTypeFor(typeName, newClass);
                inputTypeName = QLatin1String("const ") + typeName + QLatin1String("&");
            }
            if ( type->isArray() ) {
                const QString arrayTypeName = mTypeMap.localType( type->arrayType() );
                Q_ASSERT(!arrayTypeName.isEmpty());
                //qDebug() << "array of" << attribute.arrayType() << "->" << arrayTypeName;
                typeName = listTypeFor(arrayTypeName, newClass);
                newClass.addInclude(QString(), arrayTypeName); // add forward declaration
                newClass.addHeaderIncludes( QStringList() << QLatin1String("QtCore/QList") );
                inputTypeName = QLatin1String("const ") + typeName + QLatin1Char('&');
            }

            // member variables
            KODE::MemberVariable variable( elemIt.name(), typeName );
            addVariableInitializer( variable );
            newClass.addMemberVariable( variable );

            const QString variableName = QLatin1String("d_ptr->") + variable.name();

            const QString upperName = upperlize( elemIt.name() );
            const QString lowerName = lowerlize( elemIt.name() );

            // setter method
            KODE::Function setter( QLatin1String("set") + upperName, QLatin1String("void") );
            setter.addArgument( inputTypeName + QLatin1Char(' ') + mNameMapper.escape( lowerName ) );
            setter.setBody( variableName + QLatin1String(" = ") + mNameMapper.escape( lowerName ) + QLatin1Char(';') );

            // getter method
            KODE::Function getter( mNameMapper.escape( lowerName ), typeName );
            getter.setBody( QLatin1String("return ") + variableName + QLatin1Char(';') );
            getter.setConst( true );

            newClass.addFunction( setter );
            newClass.addFunction( getter );
        }

        // include header
        newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarations( elemIt.type() ) );
        newClass.addHeaderIncludes( mTypeMap.headerIncludes( elemIt.type() ) );
        if ( elemIt.maxOccurs() > 1 )
            newClass.addHeaderIncludes(QStringList() << QLatin1String("QtCore/QList"));
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
        const QString variableName = QLatin1String("d_ptr->") + variable.name();

        QString upperName = upperlize( attribute.name() );
        QString lowerName = lowerlize( attribute.name() );

        // setter method
        KODE::Function setter( QLatin1String("set") + upperName, QLatin1String("void") );
        setter.addArgument( inputTypeName + QLatin1Char(' ') + mNameMapper.escape( lowerName ) );
        setter.setBody( variableName + QLatin1String(" = ") + mNameMapper.escape( lowerName ) + QLatin1Char(';') );

        // getter method
        KODE::Function getter( mNameMapper.escape( lowerName ), typeName );
        getter.setBody( QLatin1String("return ") + variableName + QLatin1Char(';') );
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

    KODE::Function dtor( QLatin1Char('~') + upperlize( newClass.name() ) );
    newClass.addFunction( dtor );

    mClasses.addClass( newClass );
}

static QString namespaceString(const QString& ns)
{
    if (ns == QLatin1String("http://www.w3.org/1999/XMLSchema"))
        return QLatin1String("KDSoapNamespaceManager::xmlSchema1999()");
    if (ns == QLatin1String("http://www.w3.org/2001/XMLSchema"))
        return QLatin1String("KDSoapNamespaceManager::xmlSchema2001()");
    //qDebug() << "got namespace" << ns;
    // TODO register into KDSoapNamespaceManager? This means generating code in the clientinterface ctor...
    return QLatin1String("QString::fromLatin1(\"") + ns + QLatin1String("\")");
}

// Helper method for the generation of the serialize() method, also used in addMessageArgument.
KODE::Code Converter::serializeElementArg( const QName& type, const QName& elementType, const QName& name, const QString& localVariableName, const QByteArray& varName, bool append, bool isQualified )
{
    const QString varAndMethodBefore = varName + (append ? QLatin1String(".append(") : QLatin1String(" = "));
    const QString varAndMethodAfter = (append ? QLatin1String(")") : QString());
    KODE::Code block;
    // for debugging, add this:
    //block += "// type: " + type.qname() + " element:" + elementType.qname();

    //if ( name.localName() == "..." )
    //    qDebug() << "appendElementArg:" << name << "type=" << type << "isBuiltin=" << mTypeMap.isBuiltinType(type) << "isQualified=" << isQualified;
    if ( mTypeMap.isTypeAny( type ) ) {
        block += QLatin1String("if (!") + localVariableName + QLatin1String(".isNull()) {");
        block.indent();
        block += varAndMethodBefore + localVariableName + varAndMethodAfter + QLatin1String(";") + COMMENT;
        block.unindent();
        block += "}";
    } else {
        const QString nameArg = QLatin1String("QString::fromLatin1(\"") + name.localName() + QLatin1String("\")");
        const QString namespaceArg = namespaceString(name.nameSpace());
        const QName actualType = type.isEmpty() ? elementType : type;
        const QString typeArgs = namespaceString(actualType.nameSpace()) + QLatin1String(", QString::fromLatin1(\"") + actualType.localName() + QLatin1String("\")");
        const QString valueVarName = QLatin1String("_value") + upperlize(name.localName());
        if ( mTypeMap.isComplexType( type, elementType ) ) {
            block += QLatin1String("KDSoapValue ") + valueVarName + QLatin1Char('(') + localVariableName + QLatin1String(".serialize(") + nameArg + QLatin1String("));") + COMMENT;
        } else {
            if ( mTypeMap.isBuiltinType( type, elementType ) ) {
                const QString qtTypeName = mTypeMap.localType( type, elementType );
                const QString value = mTypeMap.serializeBuiltin( type, elementType, localVariableName, qtTypeName );

                block += QLatin1String("KDSoapValue ") + valueVarName + QLatin1String("(" )+ nameArg + QLatin1String(", ") + value + QLatin1String(", ") + typeArgs + QLatin1String(");") + COMMENT;
            } else {
                block += QLatin1String("KDSoapValue ") + valueVarName + QLatin1String("(") + nameArg + QLatin1String(", ") + localVariableName + QLatin1String(".serialize(), ") + typeArgs + QLatin1String(");") + COMMENT;
            }
        }
        block += valueVarName + QLatin1String(".setNamespaceUri(") + namespaceArg + QLatin1String(");");
        if ( isQualified )
            block += valueVarName + QLatin1String(".setQualified(true);");
        block += varAndMethodBefore + valueVarName + varAndMethodAfter + QLatin1String(";") + COMMENT;
    }
    return block;
}

// Helper method for the generation of the deserialize() method
static KODE::Code demarshalNameTest( const QName& type, const QString& tagName, bool *first )
{
    KODE::Code demarshalCode;
    if ( type.nameSpace() == XMLSchemaURI && (type.localName() == QLatin1String("any")) ) {
        demarshalCode += QString::fromLatin1(*first? "" : "else ") + QLatin1String("{") + COMMENT;
    } else {
        demarshalCode += QString::fromLatin1(*first? "" : "else ") + QLatin1String("if (name == QLatin1String(\"") + tagName + QLatin1String("\")) {") + COMMENT;
    }
    *first = false;
    return demarshalCode;
}

// Helper method for the generation of the deserialize() method, also used by convertClientCall
KODE::Code Converter::demarshalVar( const QName& type, const QName& elementType, const QString& variableName, const QString& qtTypeName, const QString& soapValueVarName ) const
{
    KODE::Code code;
    if ( mTypeMap.isTypeAny( type ) ) {
        code += variableName + QLatin1String(" = ") + soapValueVarName + QLatin1String(";") + COMMENT;
    } else if ( mTypeMap.isBuiltinType( type, elementType ) ) {
        code += variableName + QLatin1String(" = ") + mTypeMap.deserializeBuiltin(type, elementType, soapValueVarName + QLatin1String(".value()"), qtTypeName) + QLatin1String(";") + COMMENT;
    } else if ( mTypeMap.isComplexType( type, elementType ) ) {
        code += variableName + QLatin1String(".deserialize(") + soapValueVarName + QLatin1String(");") + COMMENT;
    } else {
        code += variableName + QLatin1String(".deserialize(") + soapValueVarName + QLatin1String(".value());" ) + COMMENT;
    }
    return code;
}

KODE::Code Converter::demarshalArrayVar( const QName& type, const QString& variableName, const QString& typeName ) const
{
    KODE::Code code;
    if ( mTypeMap.isTypeAny( type ) ) { // KDSoapValue doesn't support temp vars [still true?]. This special-casing is ugly though.
        code += variableName + QLatin1String(".append(val);");
    } else {
        // we need a temp var in case of deserialize()
        // [TODO: we could merge demarshalVar into this code, to avoid the temp var in other cases]
        QString tempVar;
        if (variableName.startsWith(QLatin1String("d_ptr->")))
            tempVar = variableName.mid(7) + QLatin1String("Temp");
        else
            tempVar = variableName + QLatin1String("Temp");
        code += typeName + QLatin1String(" ") + tempVar +QLatin1String( ";");
        code.addBlock( demarshalVar( type, QName(), tempVar, typeName ) );
        code += variableName + QLatin1String(".append(") + tempVar + QLatin1String(");");
    }
    return code;
}

void Converter::createComplexTypeSerializer( KODE::Class& newClass, const XSD::ComplexType *type )
{
    newClass.addInclude(QLatin1String("KDSoapClient/KDSoapNamespaceManager.h"));

    KODE::Function serializeFunc( QLatin1String("serialize"), QLatin1String("KDSoapValue") );
    serializeFunc.addArgument( QLatin1String("const QString& valueName") );
    serializeFunc.setConst( true );

    KODE::Function deserializeFunc( QLatin1String("deserialize"), QLatin1String("void") );
    deserializeFunc.addArgument( QLatin1String("const KDSoapValue& mainValue") );

    KODE::Code marshalCode, demarshalCode;

    const QString typeArgs = namespaceString(type->nameSpace()) + QLatin1String(", QString::fromLatin1(\"") + type->name() + QLatin1String("\")");

    if ( type->baseTypeName() != XmlAnyType && !type->baseTypeName().isEmpty() && !type->isArray() ) {

        const QName baseName = type->baseTypeName();
        const QString typeName = mTypeMap.localType( baseName );
        KODE::MemberVariable variable( QLatin1String("value"), typeName );
        const QString variableName = QLatin1String("d_ptr->") + variable.name();

        if ( mTypeMap.isComplexType( baseName ) ) {
            marshalCode += QLatin1String("KDSoapValue mainValue = ") + variableName + QLatin1String(".serialize(valueName);") + COMMENT;
            marshalCode += QLatin1String("mainValue.setType(") + typeArgs + QLatin1String(");");
        } else {
            QString value;
            if ( mTypeMap.isBuiltinType( baseName ) )
                value = mTypeMap.serializeBuiltin( baseName, QName(), variableName, typeName );
            else
                value += variableName + QLatin1String(".serialize()");
            marshalCode += QLatin1String("KDSoapValue mainValue(valueName, ") + value + QLatin1String(", ") + typeArgs + QLatin1String(");") + COMMENT;
        }

        demarshalCode += demarshalVar( baseName, QName(), variableName, typeName, QLatin1String("mainValue") );
    } else {
        marshalCode += QLatin1String("KDSoapValue mainValue(valueName, QVariant(), ") + typeArgs + QLatin1String(");") + COMMENT;
    }

    // elements
    XSD::Element::List elements = type->elements();

    // remove "void" elements (testcase: "result" in salesforce-partner's setPasswordResponse)
    QMutableListIterator<XSD::Element> itElem(elements);
    while (itElem.hasNext()) {
        const XSD::Element& elem = itElem.next();
        const QString typeName = mTypeMap.localType( elem.type() );
        if (typeName == QLatin1String("void"))
            itElem.remove();
    }
    const XSD::Attribute::List attributes = type->attributes();
    if ( !elements.isEmpty() || !attributes.isEmpty() ) {
        demarshalCode += QLatin1String("const KDSoapValueList& args = mainValue.childValues();") + COMMENT;
    }

    if ( !elements.isEmpty() ) {
        marshalCode += QLatin1String("KDSoapValueList& args = mainValue.childValues();") + COMMENT;
        if (elements.at(0).isQualified()) {
            marshalCode += QLatin1String("mainValue.setQualified(true);") + COMMENT;
        }
        demarshalCode += "for (int argNr = 0; argNr < args.count(); ++argNr) {";
        demarshalCode.indent();
        demarshalCode += "const KDSoapValue& val = args.at(argNr);";
        demarshalCode += "const QString name = val.name();";
    } else {
        // The Q_UNUSED is not necessarily true in case of attributes, but who cares.
        demarshalCode += QLatin1String("Q_UNUSED(mainValue);") + COMMENT;
    }

    if ( type->isArray() ) {
        if (elements.count() != 1) {
            qDebug() << "array" << type->name() << "has" << elements.count() << "elements!";
        }
        Q_ASSERT(elements.count() == 1);
        const XSD::Element elem = elements.first();
        //const QString typeName = mTypeMap.localType( elem.type() );
        //Q_ASSERT(!typeName.isEmpty());
        KODE::MemberVariable variable( elem.name(), QLatin1String("whatever") ); // was already added; this is just for the naming
        const QString variableName = QLatin1String("d_ptr->") + variable.name(); // always d_ptr->mEntries, actually
        const QName arrayType = type->arrayType();
        const QString typeName = mTypeMap.localType( arrayType );

        marshalCode += QLatin1String("args.setArrayType(QString::fromLatin1(\"") + arrayType.nameSpace() + QLatin1String("\"), QString::fromLatin1(\"") + arrayType.localName() + QLatin1String("\"));");
        marshalCode += QLatin1String("for (int i = 0; i < ") + variableName + QLatin1String(".count(); ++i) {");
        marshalCode.indent();
        const QString nameSpace = elem.nameSpace();
        marshalCode.addBlock( serializeElementArg( arrayType, QName(), QName(nameSpace, QLatin1String("item")), variableName + QLatin1String(".at(i)"), "args", true, elem.isQualified() ) );
        marshalCode.unindent();
        marshalCode += '}';

        demarshalCode.addBlock( demarshalArrayVar( arrayType, variableName, typeName ) );
    } else {
        bool first = true;
        Q_FOREACH( const XSD::Element& elem, elements ) {

            const QString elemName = elem.name();
            const QString typeName = mTypeMap.localType( elem.type() );
            Q_ASSERT(!typeName.isEmpty());
            Q_ASSERT(typeName != QLatin1String("void")); // removed earlier in this file

            KODE::MemberVariable variable( elemName, typeName ); // was already added; this is just for the naming
            const QString variableName = QLatin1String("d_ptr->") + variable.name();

            demarshalCode.addBlock( demarshalNameTest( elem.type(), elemName, &first ) );
            demarshalCode.indent();

            if ( elem.maxOccurs() > 1 ) {
                //const QString typePrefix = mNSManager.prefix( elem.type().nameSpace() );

                marshalCode += QLatin1String("for (int i = 0; i < ") + variableName + QLatin1String(".count(); ++i) {");
                marshalCode.indent();
                marshalCode.addBlock( serializeElementArg( elem.type(), QName(), elem.qualifiedName(), variableName + QLatin1String(".at(i)"), "args", true, elem.isQualified() ) );
                marshalCode.unindent();
                marshalCode += '}';

                demarshalCode.addBlock( demarshalArrayVar( elem.type(), variableName, typeName ) );
            } else {
                marshalCode.addBlock( serializeElementArg( elem.type(), QName(), elem.qualifiedName(), variableName, "args", true, elem.isQualified() ) );
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
            KODE::MemberVariable variable( attrName, QLatin1String("doesnotmatter") ); // was already added; this is just for the naming
            const QString variableName = QLatin1String("d_ptr->") + variable.name();

            demarshalCode.addBlock( demarshalNameTest( attribute.type(), attrName, &first ) );
            demarshalCode.indent();

            marshalCode.addBlock( serializeElementArg( attribute.type(), QName(), attribute.qualifiedName(), variableName, "attribs", true, attribute.isQualified() ) );

            const QString typeName = mTypeMap.localType( attribute.type() );
            Q_ASSERT(!typeName.isEmpty());
            demarshalCode.addBlock( demarshalVar( attribute.type(), QName(), variableName, typeName ) );

            demarshalCode.unindent();
            demarshalCode += "}";
        }
        marshalCode += QLatin1String("mainValue.childValues().attributes() += attribs;") + COMMENT;

        demarshalCode.unindent();
        demarshalCode += "}";
    }

    marshalCode += "return mainValue;";

    serializeFunc.setBody( marshalCode );
    newClass.addFunction( serializeFunc );

    deserializeFunc.setBody( demarshalCode );
    newClass.addFunction( deserializeFunc );
}
