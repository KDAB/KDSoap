#include "converter.h"
#include <QSet>

using namespace KWSDL;

void Converter::convertServerService()
{
    KODE::Class serverClass("KDWSDLServerClass");

    KODE::Function processRequestMethod(QString::fromLatin1("processRequest"), QString::fromLatin1("void"));
    processRequestMethod.addArgument("const KDSoapMessage &request");
    processRequestMethod.addArgument("KDSoapMessage &response");

    KODE::Code body;
    body.addLine("const QByteArray method = request.name().toLatin1();");

    bool first = true;
    QSet<QName> uniqueBindings = mWSDL.uniqueBindings();
    Q_FOREACH( const QName& bindingName, uniqueBindings ) {
        const Binding binding = mWSDL.findBinding( bindingName );

        PortType portType = mWSDL.findPortType( binding.portTypeName() );
        //qDebug() << portType.name();
        const Operation::List operations = portType.operations();
        Q_FOREACH( const Operation& operation, operations ) {
            const Operation::OperationType opType = operation.operationType();
            switch(opType) {
            case Operation::OneWayOperation:
            case Operation::RequestResponseOperation: // the standard case
            case Operation::SolicitResponseOperation:
            case Operation::NotificationOperation:
                generateServerMethod(body, binding, operation, serverClass, first);
                break;
            }
            first = false;
        }
    }

    if (!first) {
        body += "else {";
        body.indent();
    }
    body += "KDSoapServerObjectInterface::processRequest(request, response);";
    if (!first) {
        body.unindent();
        body += "}";
    }
    processRequestMethod.setBody(body);

    serverClass.addFunction(processRequestMethod);

    mClasses += serverClass;
}

void Converter::generateServerMethod(KODE::Code& code, const Binding& binding, const Operation& operation, KODE::Class &newClass, bool first)
{
    const Message message = mWSDL.findMessage( operation.input().message() );
    const Message outputMessage = mWSDL.findMessage( operation.output().message() );

    const QString operationName = operation.name();
    code += QString(first ? "" : "else ") + "if (method == \"" + operationName + "\") {";
    code.indent();

    QStringList inputVars;
    const Part::List parts = message.parts();
    if (parts.count() > 1) {
        qWarning("ERROR: multiple input parameters are not supported - please report this with your wsdl file to kdsoap-support@kdab.com");
    }
    Q_FOREACH( const Part& part, parts ) {
        const QString lowerName = lowerlize( part.name() );
        const QString argType = mTypeMap.localType( part.type(), part.element() );
        //qDebug() << "localInputType" << part.type().qname() << part.element().qname() << "->" << argType;
        if ( argType != "void" ) {
            const QString varName = mNameMapper.escape( lowerName );

            code += argType + ' ' + varName + ";" COMMENT;

            // what if there's more than one?
            code.addBlock( demarshalVar( part.type(), part.element(), varName, argType, "request" ) );

            inputVars += varName;
            newClass.addIncludes( mTypeMap.headerIncludes( part.type() ) );
        }
    }

    const Part::List outParts = outputMessage.parts();
    if (outParts.count() > 1) {
        qWarning("ERROR: multiple output parameters are not supported - please report this with your wsdl file to kdsoap-support@kdab.com");
    } else if (outParts.isEmpty()) {
        code += operationName + '(' + inputVars.join(", ") + ");";
    } else {
        QString retType;
        bool isBuiltin = false;
        bool isComplex = false;
        Part retPart;
        Q_FOREACH( const Part& outPart, outParts ) {
            retType = mTypeMap.localType( outPart.type(), outPart.element() );
            isBuiltin = mTypeMap.isBuiltinType( outPart.type(), outPart.element() );
            isComplex = mTypeMap.isComplexType( outPart.type(), outPart.element() );
            retPart = outPart;
        }
        const QString methodName = lowerlize( operation.name() );
        const QString functionName = mNameMapper.escape( methodName );
        code += retType + " ret = " + functionName + '(' + inputVars.join(", ") + ");" COMMENT;
        code += "if (!hasFault()) {";
        code.indent();
        // basic type: response.setValue(ret);
        // complex type: response.childValues() += ret.serialize(QString()).childValues();
        addMessageArgument( code, soapStyle(binding), retPart, "ret", "response" );
        code.unindent();
        code += "}";
    }
    code.unindent();
    code += "}";
}
