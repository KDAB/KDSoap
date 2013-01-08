#include "converter.h"
#include "settings.h"
#include <libkode/style.h>
#include <QSet>
#include <QDebug>

using namespace KWSDL;

void Converter::convertServerService()
{
    Q_FOREACH( const Service& service, mWSDL.definitions().services() ) {
        Q_ASSERT(!service.name().isEmpty());

        QSet<QName> uniqueBindings = mWSDL.uniqueBindings( service );

        Q_FOREACH( const QName& bindingName, uniqueBindings ) {
            //qDebug() << "binding" << bindingName;
            const Binding binding = mWSDL.findBinding( bindingName );

            QString className = KODE::Style::className(service.name());
            QString nameSpace;
            if (uniqueBindings.count() > 1) {
                // Multiple bindings: use Service::Binding as classname.
                nameSpace = className;
                className = KODE::Style::className(bindingName.localName());
            }
            className += "ServerBase";

            KODE::Class serverClass(className, nameSpace);
            serverClass.addBaseClass(mQObject);
            serverClass.addBaseClass(mKDSoapServerObjectInterface);
            if (!Settings::self()->exportDeclaration().isEmpty())
                serverClass.setExportDeclaration(Settings::self()->exportDeclaration());
            serverClass.setNameSpace( Settings::self()->nameSpace() );

            // Files included in the header
            serverClass.addHeaderInclude("QtCore/QObject");
            serverClass.addHeaderInclude("KDSoapServer/KDSoapServerObjectInterface.h");

            serverClass.addDeclarationMacro("Q_OBJECT");
            serverClass.addDeclarationMacro("Q_INTERFACES(KDSoapServerObjectInterface)");

            KODE::Function processRequestMethod(QString::fromLatin1("processRequest"), QString::fromLatin1("void"));
            processRequestMethod.addArgument("const KDSoapMessage &request");
            processRequestMethod.addArgument("KDSoapMessage &response");
            processRequestMethod.addArgument("const QByteArray& soapAction");

            KODE::Code body;
            const QString responseNs = mWSDL.definitions().targetNamespace();
            body.addLine("setResponseNamespace(QLatin1String(\"" + responseNs + "\"));" + COMMENT);
            body.addLine("const QByteArray method = request.name().toLatin1();");

            PortType portType = mWSDL.findPortType( binding.portTypeName() );
            //qDebug() << portType.name();
            bool first = true;
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
            body += "KDSoapServerObjectInterface::processRequest(request, response, soapAction);"  + COMMENT;
            if (!first) {
                body.unindent();
                body += "}";
            }
            processRequestMethod.setBody(body);

            serverClass.addFunction(processRequestMethod);

            mClasses.addClass(serverClass);
        }
    }
}

void Converter::generateServerMethod(KODE::Code& code, const Binding& binding, const Operation& operation, KODE::Class &newClass, bool first)
{
    const Message message = mWSDL.findMessage( operation.input().message() );
    Message outputMessage;
    if (operation.operationType() != Operation::OneWayOperation) {
        outputMessage = mWSDL.findMessage( operation.output().message() );
    }

    const QString operationName = operation.name();
    const QString methodName = mNameMapper.escape( lowerlize( operationName ) );

    KODE::Function virtualMethod(methodName);
    virtualMethod.setVirtualMode(KODE::Function::PureVirtual);

    QString condition = "method == \"" + operationName + "\"";
    if ( binding.type() == Binding::SOAPBinding ) {
        const SoapBinding soapBinding( binding.soapBinding() );
        const SoapBinding::Operation op = soapBinding.operations().value( operation.name() );
        if (!op.action().isEmpty()) {
            condition += "|| soapAction == \"" + op.action() + "\"";
        }
    }
    code += QString(first ? "" : "else ") + "if (" + condition + ") {";
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

            code += argType + ' ' + varName + ";" + COMMENT;

            QString soapValueVarName = "request";
            if (soapStyle(binding) == SoapBinding::RPCStyle) {
                // RPC comes with a wrapper element, dig into it here
                code += QLatin1String("const KDSoapValue val = request.childValues().first();") + COMMENT;
                soapValueVarName = "val";
            }

            // what if there's more than one?
            code.addBlock( demarshalVar( part.type(), part.element(), varName, argType, soapValueVarName ) );

            inputVars += varName;
            newClass.addIncludes( mTypeMap.headerIncludes( part.type() ) );
            virtualMethod.addArgument( mTypeMap.localInputType( part.type(), part.element() ) + ' ' + varName );
        }
    }

    const Part::List outParts = outputMessage.parts();
    if (outParts.count() > 1) {
        qWarning("ERROR: multiple output parameters are not supported (operation %s) - please report"
                 " this with your wsdl file to kdsoap-support@kdab.com", qPrintable(operation.name()));
        virtualMethod.setReturnType("void /*UNSUPPORTED*/");
    } else if (outParts.isEmpty()) {
        code += operationName + '(' + inputVars.join(", ") + ");";
        virtualMethod.setReturnType("void");
    } else {
        QString retType;
        QString retInputType;
        //bool isBuiltin = false;
        //bool isComplex = false;
        Part retPart;
        Q_FOREACH( const Part& outPart, outParts /* only one */ ) {
            retType = mTypeMap.localType( outPart.type(), outPart.element() );
            retInputType = mTypeMap.localInputType( outPart.type(), outPart.element() );
            //isBuiltin = mTypeMap.isBuiltinType( outPart.type(), outPart.element() );
            //isComplex = mTypeMap.isComplexType( outPart.type(), outPart.element() );
            retPart = outPart;
        }
        const QString methodCall = methodName + '(' + inputVars.join(", ") + ')';
        if (retType == "void") {
            code += methodCall + ";" + COMMENT;
        } else {
            code += retType + " ret = " + methodCall + ";" + COMMENT;
        }
        code += "if (!hasFault()) {";
        code.indent();

        bool qualified;
        const QName elemName = elementNameForPart( retPart, &qualified );

        if (soapStyle(binding) == SoapBinding::RPCStyle) {
            code += QString("KDSoapValue wrapper(\"%1\", QVariant());").arg(outputMessage.name());
            code.addBlock( serializeElementArg( retPart.type(), retPart.element(), elemName, "ret", "wrapper.childValues()", true, qualified ) );
            code += "response = wrapper;";
        } else {
            code.addBlock( serializeElementArg( retPart.type(), retPart.element(), elemName, "ret", "response", false, qualified ) );
        }

        code.unindent();
        code += "}";
        Q_ASSERT(!retType.isEmpty());
        virtualMethod.setReturnType(retType);

        generateDelayedReponseMethod(methodName, retInputType, retPart, newClass, binding, outputMessage);
    }
    code.unindent();
    code += "}";

    newClass.addFunction(virtualMethod);
}

void Converter::generateDelayedReponseMethod(const QString& methodName, const QString& retInputType, const Part &retPart, KODE::Class &newClass,
                                             const Binding& binding, const Message &outputMessage)
{
    const QString delayedMethodName = methodName + "Response";
    KODE::Function delayedMethod(delayedMethodName);
    delayedMethod.addArgument("const KDSoapDelayedResponseHandle& responseHandle");
    delayedMethod.addArgument(retInputType + " ret");
    delayedMethod.setReturnType("void");

    KODE::Code code;
    code.addLine("KDSoapMessage response;");

    bool qualified;
    const QName elemName = elementNameForPart( retPart, &qualified );
    if (soapStyle(binding) == SoapBinding::RPCStyle) {
        code += QString("KDSoapValue wrapper(\"%1\", QVariant(), \"%2\");").arg(outputMessage.name()).arg(outputMessage.nameSpace());
        code.addBlock( serializeElementArg( retPart.type(), retPart.element(), elemName, "ret", "wrapper.childValues()", true, qualified ) );
        code += "response = wrapper;";
    } else {
        code.addBlock(serializeElementArg(retPart.type(), retPart.element(), elemName, "ret", "response", false, qualified));
    }

    code.addLine("sendDelayedResponse(responseHandle, response);");
    delayedMethod.setBody(code);

    newClass.addFunction(delayedMethod);
}
