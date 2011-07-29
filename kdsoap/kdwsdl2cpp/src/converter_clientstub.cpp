#include "converter.h"
#include <libkode/style.h>
#include "settings.h"
#include <QDebug>
#include <QCoreApplication>

using namespace KWSDL;

SoapBinding::Style Converter::soapStyle( const Binding& binding ) const
{
    if ( binding.type() == Binding::SOAPBinding ) {
        const SoapBinding soapBinding( binding.soapBinding() );
        return soapBinding.binding().style();
    }
    return SoapBinding::RPCStyle;
}


static SoapBinding::Headers getHeaders( const Binding& binding, const QString& operationName )
{
    if ( binding.type() == Binding::SOAPBinding ) {
        const SoapBinding soapBinding( binding.soapBinding() );
        const SoapBinding::Operation op = soapBinding.operations().value( operationName );
        return op.inputHeaders();
    }
    return SoapBinding::Headers();
}

bool Converter::convertClientService()
{
  const Service service = mWSDL.definitions().service();
  Q_ASSERT(!service.name().isEmpty());

  QSet<QName> uniqueBindings = mWSDL.uniqueBindings();
  //qDebug() << uniqueBindings;

  KODE::Class::List bindingClasses;
  Q_FOREACH( const QName& bindingName, uniqueBindings ) {
      const Binding binding = mWSDL.findBinding( bindingName );

      QString className = KODE::Style::className(service.name());
      QString nameSpace;
      if (uniqueBindings.count() > 1) {
          nameSpace = className;
          className = KODE::Style::className(bindingName.localName());
      }

      KODE::Class newClass( className, nameSpace );
      if (!Settings::self()->exportDeclaration().isEmpty())
        newClass.setExportDeclaration(Settings::self()->exportDeclaration());
      newClass.setUseDPointer( true, "d_ptr" /*avoid clash with possible d() method*/ );
      newClass.addBaseClass( mQObject );
      newClass.setDocs(service.documentation());

      // Files included in the header
      newClass.addHeaderInclude( "QObject" );
      newClass.addHeaderInclude( "QString" );
      newClass.addHeaderInclude("KDSoapClientInterface.h");

      // Files included in the impl, with optional forward-declarations in the header
      newClass.addInclude("KDSoapMessage.h", "KDSoapMessage");
      newClass.addInclude("KDSoapValue.h", "KDSoapValue");
      newClass.addInclude("KDSoapPendingCallWatcher.h", "KDSoapPendingCallWatcher");
      newClass.addInclude("KDSoapNamespaceManager.h");

      // Variables (which will go into the d pointer)
      KODE::MemberVariable clientInterfaceVar("m_clientInterface", "KDSoapClientInterface*");
      clientInterfaceVar.setInitializer("NULL");
      newClass.addMemberVariable(clientInterfaceVar);

      KODE::MemberVariable lastReply("m_lastReply", "KDSoapMessage");
      newClass.addMemberVariable(lastReply);

      KODE::MemberVariable endPoint("m_endPoint", "QString");
      newClass.addMemberVariable(endPoint);

      // Ctor and dtor
      {
          KODE::Function ctor( newClass.name() );
          ctor.addArgument(KODE::Function::Argument("QObject* parent", "0"));
          ctor.addInitializer("QObject(parent)");
          KODE::Function dtor( '~' + newClass.name() );
          KODE::Code ctorCode, dtorCode;

          ctor.setBody( ctorCode );
          newClass.addFunction( ctor );

          dtorCode += "delete d_ptr->m_clientInterface;";

          dtor.setBody( dtorCode );
          newClass.addFunction( dtor );
      }
      // ignoreSslErrors() method
      {
          KODE::Function ignoreSslErrors("ignoreSslErrors", "void");
          KODE::Code code;
          code += "clientInterface()->ignoreSslErrors();";
          ignoreSslErrors.setBody(code);
          ignoreSslErrors.setDocs("Asks Qt to ignore ssl errors in https requests. Use this for testing only!");
          newClass.addFunction(ignoreSslErrors);
      }
      // setEndPoint() method
      {
          KODE::Function setEndPoint("setEndPoint", "void");
          setEndPoint.addArgument( "const QString& endPoint" );
          KODE::Code code;
          code += "d_ptr->m_endPoint = endPoint;";
          setEndPoint.setBody(code);
          setEndPoint.setDocs("Overwrite the end point defined in the .wsdl file, with another http/https URL.");
          newClass.addFunction(setEndPoint);
      }
      //setSoapVersion() method
      {
	  KODE::Function setSoapVersion("setSoapVersion", "void");
	  setSoapVersion.addArgument("KDSoapClientInterface::SoapVersion soapVersion");
	  KODE::Code code;
	  code += "clientInterface()->setSoapVersion(soapVersion);";
	  setSoapVersion.setBody(code);
	  setSoapVersion.setDocs("Overwrite the soap version defined in the .wsdl file, with another version. \n"
				"version can be KDSoapClientInterface::SOAP1_1 or KDSoapClientInterface::SOAP1_2");
	  newClass.addFunction(setSoapVersion);
      }
      // lastError() method
      {
          KODE::Function lastError("lastError", "QString");
          lastError.setConst(true);
          KODE::Code code;
          code += "if (d_ptr->m_lastReply.isFault())";
          code.indent();
          code += "return d_ptr->m_lastReply.faultAsString();";
          code.unindent();
          code += "return QString();";
          lastError.setBody(code);
          lastError.setDocs("Return the error from the last blocking call.\nEmpty if no error.");
          newClass.addFunction(lastError);
      }
      //soapError() signal
      {
          KODE::Function errorSignal( "soapError", "void", KODE::Function::Signal );
          errorSignal.addArgument( "const QString& method" );
          errorSignal.addArgument( "const KDSoapMessage& fault" );
          errorSignal.setDocs( "This signal is emitted whenever a SOAP call failed, for a central processing of all SOAP errors.\nmethod is the name of the method (or operation) that returned the fault, for instance \"addContact\"." );
          newClass.addFunction(errorSignal);
      }

      QUrl webserviceLocation;

      if ( binding.type() == Binding::SOAPBinding ) {
          const SoapBinding soapBinding( binding.soapBinding() );
          const SoapBinding::Address address = soapBinding.address();
          if ( address.location().isValid() )
              webserviceLocation = address.location();
      } else {
          // ignore non-SOAP bindings, like HTTP GET and HTTP POST
          continue;
      }

      // clientInterface() private method
      {
          KODE::Function clientInterface("clientInterface", "KDSoapClientInterface*", KODE::Function::Private);
          KODE::Code code;
          code += "if (!d_ptr->m_clientInterface) {";
          code.indent();
          const QByteArray encoded = webserviceLocation.toEncoded();
          code += "const QString endPoint = !d_ptr->m_endPoint.isEmpty() ? d_ptr->m_endPoint : QString::fromLatin1(\"" + QString::fromLatin1( encoded.data(), encoded.size() ) + "\");";
          code += "const QString messageNamespace = QString::fromLatin1(\"" + mWSDL.definitions().targetNamespace() + "\");";
          code += "d_ptr->m_clientInterface = new KDSoapClientInterface(endPoint, messageNamespace);";
          if ( soapStyle(binding) == SoapBinding::DocumentStyle ) {
              code += "d_ptr->m_clientInterface->setStyle( KDSoapClientInterface::DocumentStyle );";
          }
          code += "d_ptr->m_clientInterface->setSoapVersion( KDSoapClientInterface::SOAP1_1 );";
          code.unindent();
          code += "}";
          code += "return d_ptr->m_clientInterface;";
          clientInterface.setBody(code);
          newClass.addFunction(clientInterface);
      }

      SoapBinding::Headers soapHeaders;

      PortType portType = mWSDL.findPortType( binding.portTypeName() );
      //qDebug() << portType.name();
      const Operation::List operations = portType.operations();
      Q_FOREACH( const Operation& operation, operations ) {
          Operation::OperationType opType = operation.operationType();
          switch(opType) {
          case Operation::OneWayOperation:
              convertClientInputMessage( operation, binding, newClass );
              break;
          case Operation::RequestResponseOperation: // the standard case
              // sync method
              if (!convertClientCall( operation, binding, newClass )) {
                  return false;
              }
              // async method
              convertClientInputMessage( operation, binding, newClass );
              convertClientOutputMessage( operation, binding, newClass );
              // TODO fault
              break;
          case Operation::SolicitResponseOperation:
              convertClientOutputMessage( operation, binding, newClass );
              convertClientInputMessage( operation, binding, newClass );
              // TODO fault
              break;
          case Operation::NotificationOperation:
              convertClientOutputMessage( operation, binding, newClass );
              break;
          }

          // Collect message parts used as headers
          Q_FOREACH( const SoapBinding::Header& header, getHeaders(binding, operation.name()) ) {
              if ( !soapHeaders.contains(header) )
                  soapHeaders.append( header );
          }
      } // end of for each operation

      Q_FOREACH( const SoapBinding::Header& header, soapHeaders ) {
          createHeader( header, newClass );
      }
      bindingClasses.append(newClass);

  } // end of for each port

  // First sort all classes so that the order compiles
  QStringList excludedClasses;
  excludedClasses << "KDSoapServerObjectInterface";
  mClasses.sortByDependencies(excludedClasses);
  // Then add the service, at the end

  mClasses += bindingClasses;
  return true;
}

void Converter::clientAddOneArgument( KODE::Function& callFunc, const Part& part, KODE::Class &newClass )
{
    const QString lowerName = lowerlize( part.name() );
    const QString argType = mTypeMap.localInputType( part.type(), part.element() );
    //qDebug() << "localInputType" << part.type().qname() << part.element().qname() << "->" << argType;
    if ( argType != "void" ) {
        QString def;
        if ( part.type().isEmpty() ) { // element (document style)
            // If the complex type is empty, we need it for serialization,
            // but we don't need the app to see it, so give it a default value.
            // Example:
            // TNS__LogoutResponse logout( const TNS__Logout& parameters = TNS__Logout() );
            const XSD::Element element = mWSDL.findElement( part.element() );
            const XSD::ComplexType ctype = mWSDL.findComplexType( element.type() );
            if ( !ctype.isNull() && ctype.isEmpty() ) {
                def = mTypeMap.localType( part.type(), part.element() ) + "()";
                //def += "/* " + element.type().qname() + " is empty */";
            }
        }

        KODE::Function::Argument arg( argType + ' ' + mNameMapper.escape( lowerName ), def );
        callFunc.addArgument( arg );
    }
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( part.type() ) );
}

static Part::List selectedParts( const Binding& binding, const Message& message, const Operation& operation, bool input )
{
    QString selectedPart;
    if ( binding.type() == Binding::SOAPBinding ) {
        const SoapBinding soapBinding = binding.soapBinding();
        const SoapBinding::Operation op = soapBinding.operations().value( operation.name() );
        selectedPart = input ? op.input().part() : op.output().part();
        if (!selectedPart.isEmpty()) {
            Part::List selected;
            Q_FOREACH( const Part& part, message.parts() ) {
                if ( part.name() == selectedPart ) { // support for <soap:body parts="MoveFolderResult"/> (msexchange)
                    selected << part;
                }
            }
            return selected;
        }
    }

    return message.parts();
}

void Converter::clientAddArguments( KODE::Function& callFunc, const Message& message, KODE::Class &newClass, const Operation &operation, const Binding &binding )
{
    const Part::List parts = selectedParts( binding, message, operation, true /*input*/ );
    Q_FOREACH( const Part& part, parts ) {
        clientAddOneArgument( callFunc, part, newClass );
    }
}

bool Converter::clientAddAction( KODE::Code& code, const Binding &binding, const QString& operationName )
{
    bool hasAction = false;
    if ( binding.type() == Binding::SOAPBinding ) {
        const SoapBinding soapBinding( binding.soapBinding() );
        const SoapBinding::Operation op = soapBinding.operations().value( operationName );
        if (!op.action().isEmpty()) {
            code += "const QString action = QString::fromLatin1(\"" + op.action() + "\");";
            hasAction = true;
        }
    }
    return hasAction;
}

QString Converter::elementNameForPart(const Part& part) const
{
    if (part.type().isEmpty()) { // element (document style)
        XSD::Element element = mWSDL.findElement(part.element());
        return element.name();
    } else { // type (rpc style)
        return part.name();
    }
}

void Converter::addMessageArgument( KODE::Code& code, const SoapBinding::Style& bindingStyle, const Part& part, const QString& localVariableName, const QByteArray& messageName )
{
    const QString lowerName = lowerlize( localVariableName );
    // In document style, the "part" is directly added as arguments
    // See http://www.ibm.com/developerworks/webservices/library/ws-whichwsdl/
    if ( bindingStyle == SoapBinding::DocumentStyle )
        code.addBlock( serializeElementArg( part.type(), part.element(), elementNameForPart(part), lowerName, messageName, false) );
    else {
        const QString argType = mTypeMap.localType( part.type(), part.element() );
        if ( argType != "void" ) {
            code.addBlock( serializeElementArg( part.type(), part.element(), elementNameForPart(part), lowerName, messageName + ".childValues()", true ) );
        }
    }
}

void Converter::clientGenerateMessage( KODE::Code& code, const Binding& binding, const Message& message, const Operation& operation )
{
    code += "KDSoapMessage message;";

    if ( binding.type() == Binding::SOAPBinding ) {
        const SoapBinding soapBinding = binding.soapBinding();
        const SoapBinding::Operation op = soapBinding.operations().value( operation.name() );
        if ( op.input().use() == SoapBinding::EncodedUse )
            code += "message.setUse(KDSoapMessage::EncodedUse);";
        else
            code += "message.setUse(KDSoapMessage::LiteralUse);";
        //qDebug() << "input headers:" << op.inputHeaders().count();
    }

    Q_FOREACH( const Part& part, selectedParts( binding, message, operation, true /*input*/ ) ) {
        addMessageArgument( code, soapStyle(binding), part, part.name(), "message" );
    }
}

KODE::Code Converter::deserializeRetVal(const KWSDL::Part& part, const QString& replyMsgName, const QString& qtRetType, const QString& varName) const
{
    // This is the opposite logic as appendElementArg, which does:
    // if builtin -> xml element with basic contents
    // if complex -> serialize and add result
    // else -> serialize

    KODE::Code code;
    const bool isBuiltin = mTypeMap.isBuiltinType( part.type(), part.element() );
    const bool isComplex = mTypeMap.isComplexType( part.type(), part.element() );
    if ( isBuiltin ) {
        code += varName + " = " + mTypeMap.deserializeBuiltin( part.type(), part.element(), replyMsgName + ".value()", qtRetType ) + ";" COMMENT;
    } else if ( isComplex ) {
        code += varName + ".deserialize(" + replyMsgName + ");" COMMENT;
    } else {
        // testcase: MyWsdlDocument::sendTelegram
        code += varName + ".deserialize(" + replyMsgName + ".value());" COMMENT;
    }
    return code;
}

// Generate synchronous call
bool Converter::convertClientCall( const Operation &operation, const Binding &binding, KODE::Class &newClass )
{
  const QString methodName = lowerlize( operation.name() );
  KODE::Function callFunc( mNameMapper.escape( methodName ), "void", KODE::Function::Public );
  callFunc.setDocs(QString("Blocking call to %1.\nNot recommended in a GUI thread.").arg(operation.name()));
  const Message inputMessage = mWSDL.findMessage( operation.input().message() );
  const Message outputMessage = mWSDL.findMessage( operation.output().message() );
  clientAddArguments( callFunc, inputMessage, newClass, operation, binding );
  KODE::Code code;
  const bool hasAction = clientAddAction( code, binding, operation.name() );
  clientGenerateMessage( code, binding, inputMessage, operation );
  QString callLine = "d_ptr->m_lastReply = clientInterface()->call(QLatin1String(\"" + operation.name() + "\"), message";
  if (hasAction) {
      callLine += ", action";
  }
  callLine += ");";
  code += callLine;

  // Return value(s) :
  const Part::List outParts = selectedParts( binding, outputMessage, operation, false /*output*/ );
  const bool singleReturnValue = outParts.count() == 1;

  if (singleReturnValue) {
      const Part retPart = outParts.first();
      const QString retType = mTypeMap.localType( retPart.type(), retPart.element() );
      if ( retType.isEmpty() ) {
          qWarning("Could not generate operation '%s'", qPrintable(operation.name()));
          return false;
      }
      callFunc.setReturnType( retType );

      code += "if (d_ptr->m_lastReply.isFault())";
      code.indent();
      code += "return " + retType + "();"; // default-constructed value
      code.unindent();

      // WARNING: if you change the logic below, also adapt the result parsing for async calls

      if ( retType != "void" ) {
          if ( soapStyle(binding) == SoapBinding::DocumentStyle /*no wrapper*/ ) {
              code += retType + " ret;"; // local var
              code.addBlock(deserializeRetVal(retPart, "d_ptr->m_lastReply", retType, "ret"));
              code += "return ret;" COMMENT;
          } else { // RPC style (adds a wrapper), or simple value
              // Protect the call to .first() below
              code += "if (d_ptr->m_lastReply.childValues().isEmpty()) {";
              code.indent();
              code += "d_ptr->m_lastReply.setFault(true);";
              code += "d_ptr->m_lastReply.addArgument(QString::fromLatin1(\"faultcode\"), QString::fromLatin1(\"Server.EmptyResponse\"));";
              code += "return " + retType + "();"; // default-constructed value
              code.unindent();
              code += "}";

              code += retType + " ret;"; // local var
              code += "const KDSoapValue val = d_ptr->m_lastReply.childValues().first();" COMMENT;
              code += demarshalVar( retPart.type(), retPart.element(), "ret", retType );
              code += "return ret;";
          }
      }

  } else {
      // Add each output argument as a non-const ref to the method. A bit ugly but no other way.

      code += "if (d_ptr->m_lastReply.isFault())";
      code.indent();
      code += "return;" COMMENT;
      code.unindent();
      Q_ASSERT(soapStyle(binding) == SoapBinding::DocumentStyle); // RPC with multiple return values? impossible, we generate a single wrapper

      Q_FOREACH(const Part& part, outParts) {
          const QString argType = mTypeMap.localType( part.type(), part.element() );
          Q_ASSERT(!argType.isEmpty());
          const QString lowerName = lowerlize( part.name() );
          KODE::Function::Argument arg( argType + "& " + mNameMapper.escape( lowerName ) );
          callFunc.addArgument( arg );
          newClass.addHeaderIncludes( mTypeMap.headerIncludes( part.type() ) );

          code.addBlock(deserializeRetVal(part, "d_ptr->m_lastReply", argType, lowerName));
      }
  }

  callFunc.setBody( code );

  newClass.addFunction( callFunc );
  return true;
}

// Generate async call method
void Converter::convertClientInputMessage( const Operation &operation,
                                           const Binding &binding, KODE::Class &newClass )
{
  QString operationName = operation.name();
  KODE::Function asyncFunc( "async" + upperlize( operationName ), "void", KODE::Function::Public );
  asyncFunc.setDocs(QString("Asynchronous call to %1.\n"
                            "Remember to connect to %2 and %3.")
                    .arg(operation.name())
                    .arg(lowerlize(operationName) + "Done")
                    .arg(lowerlize(operationName) + "Error"));
  const Message message = mWSDL.findMessage( operation.input().message() );
  clientAddArguments( asyncFunc, message, newClass, operation, binding );
  KODE::Code code;
  const bool hasAction = clientAddAction( code, binding, operation.name() );
  clientGenerateMessage( code, binding, message, operation );

  QString callLine = "KDSoapPendingCall pendingCall = clientInterface()->asyncCall(QLatin1String(\"" + operationName + "\"), message";
  if (hasAction) {
      callLine += ", action";
  }
  callLine += ");";
  code += callLine;

  if (operation.operationType() == Operation::RequestResponseOperation) {
      const QString finishedSlotName = "_kd_slot" + upperlize(operationName) + "Finished";

      code += "KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);";
      code += "connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),\n"
              "        this, SLOT(" + finishedSlotName + "(KDSoapPendingCallWatcher*)));";
      asyncFunc.setBody( code );
      newClass.addFunction( asyncFunc );
  }
}

// Generate signals and the result slot, for async calls
void Converter::convertClientOutputMessage( const Operation &operation,
                                            const Binding &binding, KODE::Class &newClass )
{
  // result signal
  QString operationName = lowerlize( operation.name() );
  KODE::Function doneSignal( operationName + "Done", "void", KODE::Function::Signal );
  doneSignal.setDocs( "This signal is emitted whenever the call to " + operationName+ "() succeeded." );

  // error signal
  KODE::Function errorSignal( operationName + "Error", "void", KODE::Function::Signal );
  errorSignal.addArgument( "const KDSoapMessage& fault" );
  errorSignal.setDocs( "This signal is emitted whenever the call to " + operationName + "() failed." );

  // finished slot
  const QString finishedSlotName = "_kd_slot" + upperlize(operationName) + "Finished";
  KODE::Function finishedSlot( finishedSlotName, "void", KODE::Function::Slot | KODE::Function::Private );
  finishedSlot.addArgument( "KDSoapPendingCallWatcher* watcher" );

  // If one output message is used by two input messages, don't define
  // it twice.
  // DF: what if the arguments are different? ...
  //if ( newClass.hasFunction( respSignal.name() ) )
  //  return;

  KODE::Code slotCode;
  slotCode += "const KDSoapMessage reply = watcher->returnMessage();";
  slotCode += "if (reply.isFault()) {";
  slotCode.indent();
  slotCode += "emit " + errorSignal.name() + "(reply);" COMMENT;
  slotCode += "emit soapError(QLatin1String(\"" + operationName + "\"), reply);";
  slotCode.unindent();
  slotCode += "} else {";
  slotCode.indent();

  const Message message = mWSDL.findMessage( operation.output().message() );

  QStringList partNames;
  const Part::List parts = selectedParts( binding, message, operation, false /*output*/ );
  Q_FOREACH( const Part& part, parts ) {
    const QString partType = mTypeMap.localType( part.type(), part.element() );
    if ( partType.isEmpty() ) {
        qWarning("Skipping part '%s'", qPrintable(part.name()));
        continue;
    }

    if ( partType == "void" )
        continue;

    QString lowerName = mNameMapper.escape( lowerlize( part.name() ) );
    doneSignal.addArgument( mTypeMap.localInputType( part.type(), part.element() ) + ' ' + lowerName );

    // WARNING: if you change the logic below, also adapt the result parsing for sync calls, above

    if ( soapStyle(binding) == SoapBinding::DocumentStyle /*no wrapper*/ ) {
        slotCode += partType + " ret;"; // local var
        slotCode.addBlock(deserializeRetVal(part, "reply", partType, "ret"));
        partNames << "ret";
    } else { // RPC style (adds a wrapper) or simple value
        QString value = "reply.childValues().child(QLatin1String(\"" + part.name() + "\"))";
        const bool isBuiltin = mTypeMap.isBuiltinType( part.type(), part.element() );
        if ( isBuiltin ) {
            partNames << value + ".value().value<" + partType + ">()";
        } else {
            slotCode += partType + " ret;"; // local var. TODO ret1/ret2 etc. if more than one.
            const bool isComplex = mTypeMap.isComplexType( part.type(), part.element() );
            if (!isComplex)
                value += ".value()";
            slotCode += "ret.deserialize(" + value + ");" COMMENT;
            partNames << "ret";
        }
    }

    // Forward declaration of element class
    //newClass.addIncludes( QStringList(), mTypeMap.forwardDeclarationsForElement( part.element() ) );
  }

  newClass.addFunction( doneSignal );
  newClass.addFunction( errorSignal );

  slotCode += "emit " + doneSignal.name() + "( " + partNames.join( "," ) + " );";
  slotCode.unindent();
  slotCode += '}';
  slotCode += "watcher->deleteLater();";

  finishedSlot.setBody(slotCode);

  newClass.addFunction(finishedSlot);
}

void Converter::createHeader( const SoapBinding::Header& header, KODE::Class &newClass )
{
    const QName messageName = header.message();
    const QString partName = header.part();
    const Message message = mWSDL.findMessage( messageName );
    const Part part = message.partByName( partName );

    {
        QString methodName = "set" + upperlize( partName );
        if (!methodName.endsWith("Header"))
            methodName += "Header";

        const QString argType = mTypeMap.localInputType( part.type(), part.element() );
        const QString methodSig = methodName + '(' + argType + ')';
        if (mHeaderMethods.contains(methodSig))
            return; // already have it (testcase: Services.wsdl)

        mHeaderMethods.insert(methodSig);
        KODE::Function headerSetter( methodName, "void", KODE::Function::Public );
        headerSetter.setDocs(QString("Sets the header '%1', for all subsequent method calls.").arg(partName));
        clientAddOneArgument( headerSetter, part, newClass );
        KODE::Code code;
        code += "KDSoapMessage message;";
        if ( header.use() == SoapBinding::EncodedUse )
            code += "message.setUse(KDSoapMessage::EncodedUse);";
        else
            code += "message.setUse(KDSoapMessage::LiteralUse);";
        addMessageArgument( code, SoapBinding::RPCStyle, part, partName, "message" );
        code += "clientInterface()->setHeader( QLatin1String(\"" + partName + "\"), message );";
        headerSetter.setBody(code);
        newClass.addFunction(headerSetter);
    }

    // Let's also generate a clear method
    {
        QString methodName = "clear" + upperlize( partName );
        if (!methodName.endsWith("Header"))
            methodName += "Header";
        KODE::Function headerClearer( methodName, "void", KODE::Function::Public );
        headerClearer.setDocs(QString("Removes the header '%1', for all subsequent method calls.").arg(partName));
        KODE::Code code;
        code += "clientInterface()->setHeader( QLatin1String(\"" + partName + "\"), KDSoapMessage() );";
        headerClearer.setBody(code);
        newClass.addFunction(headerClearer);
    }
}
