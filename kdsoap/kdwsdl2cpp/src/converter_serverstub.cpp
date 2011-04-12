#include "converter.h"
#include <libkode/style.h>
#include <QSet>
#include <QDebug>

using namespace KWSDL;

void Converter::convertServerService()
{
    const Service service = mWSDL.definitions().service();
    Q_ASSERT(!service.name().isEmpty());

    bool first = true;
    QSet<QName> uniqueBindings = mWSDL.uniqueBindings();
    Q_FOREACH( const QName& bindingName, uniqueBindings ) {
        //qDebug() << "binding" << bindingName;
        const Binding binding = mWSDL.findBinding( bindingName );

        QString className = KODE::Style::className(service.name());
        QString nameSpace;
        if (uniqueBindings.count() > 1) {
            nameSpace = className;
            className = KODE::Style::className(bindingName.localName());
        }
        className += "ServerBase";

        KODE::Class serverClass(className, nameSpace);
        serverClass.addBaseClass(mQObject);
        serverClass.addBaseClass(mKDSoapServerObjectInterface);
        // Files included in the header
        serverClass.addHeaderInclude("QObject");
        serverClass.addHeaderInclude("KDSoapServerObjectInterface.h");

        serverClass.addDeclarationMacro("Q_OBJECT");
        serverClass.addDeclarationMacro("Q_INTERFACES(KDSoapServerObjectInterface)");

        KODE::Function processRequestMethod(QString::fromLatin1("processRequest"), QString::fromLatin1("void"));
        processRequestMethod.addArgument("const KDSoapMessage &request");
        processRequestMethod.addArgument("KDSoapMessage &response");

        KODE::Code body;
        body.addLine("const QByteArray method = request.name().toLatin1();");

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
}

void Converter::generateServerMethod(KODE::Code& code, const Binding& binding, const Operation& operation, KODE::Class &newClass, bool first)
{
    const Message message = mWSDL.findMessage( operation.input().message() );
    const Message outputMessage = mWSDL.findMessage( operation.output().message() );

    const QString operationName = operation.name();
    const QString methodName = mNameMapper.escape( lowerlize( operationName ) );

    KODE::Function virtualMethod(methodName);
    virtualMethod.setVirtualMode(KODE::Function::PureVirtual);

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
            virtualMethod.addArgument( mTypeMap.localInputType( part.type(), part.element() ) + ' ' + varName );
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
        const QString methodCall = methodName + '(' + inputVars.join(", ") + ')';
        if (retType == "void") {
            code += methodCall + ";" COMMENT;
        } else {
            code += retType + " ret = " + methodCall + ";" COMMENT;
        }
        code += "if (!hasFault()) {";
        code.indent();
        // basic type: response.setValue(ret);
        // complex type:
        //   response.setValue(QLatin1String("getEmployeeCountryResponse"));
        //   response.childValues() += ret.serialize(QString()).childValues();
        //      == response.setValue(ret.serialize("getEmployeeCountryResponse")) I think.
        Q_UNUSED(binding); // need to make up fooResponse in RPC mode
        Q_ASSERT(soapStyle(binding) == SoapBinding::DocumentStyle); // TODO implement RPC!
        code.addBlock( serializeElementArg( retPart.type(), retPart.element(), elementNameForPart(retPart), "ret", "response", false ) );

        code.unindent();
        code += "}";
        virtualMethod.setReturnType(retType);
    }
    code.unindent();
    code += "}";

    newClass.addFunction(virtualMethod);
}
