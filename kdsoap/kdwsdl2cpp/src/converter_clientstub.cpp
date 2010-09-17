#include "converter.h"
#include <libkode/style.h>
#include <QDebug>

using namespace KWSDL;

static SoapBinding::Style soapStyle( const Binding& binding )
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

void Converter::convertClientService()
{
  const Service service = mWSDL.definitions().service();
  Q_ASSERT(!service.name().isEmpty());

  QSet<QName> uniqueBindings;
  Q_FOREACH( const Port& port, service.ports() ) {
      const Binding binding = mWSDL.findBinding( port.bindingName() );
      if ( binding.type() == Binding::SOAPBinding ) {
          uniqueBindings.insert( port.bindingName() );
      } else {
          // ignore non-SOAP bindings, like HTTP GET and HTTP POST
          continue;
      }
  }

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
      newClass.setUseDPointer( true, "d_ptr" /*avoid clash with possible d() method*/ );
      newClass.addBaseClass( mQObject );
      newClass.setDocs(service.documentation());

      // Files included in the header
      newClass.addHeaderInclude( "QObject" );
      newClass.addHeaderInclude( "QString" );

      // Files included in the impl, with optional forward-declarations in the header
      newClass.addInclude("KDSoapMessage.h", "KDSoapMessage");
      newClass.addInclude("KDSoapValue.h", "KDSoapValueList");
      newClass.addInclude("KDSoapValue.h", "KDSoapValue");
      newClass.addInclude("KDSoapClientInterface.h", "KDSoapClientInterface");
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
	  setSoapVersion.addArgument("int soapVersion");
	  KODE::Code code;
	  code += "if (soapVersion == 1){";
	  code.indent();
	  code += "clientInterface()->setSoapVersion(KDSoapClientInterface::SOAP1_1);";
	  code.unindent();
	  code += "}else{";
	  code.indent();
	  code += "clientInterface()->setSoapVersion(KDSoapClientInterface::SOAP1_2);";
	  code.unindent();
	  code += "}";
	  setSoapVersion.setBody(code);
	  setSoapVersion.setDocs("Overwrite the soap version defined in the .wsdl file, with another version. \n"
				"version can be 1 for soap 1.1 or 2 for soap 1.2");
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
          code += "const QString endPoint = !d_ptr->m_endPoint.isEmpty() ? d_ptr->m_endPoint : QString::fromLatin1(\"" + QLatin1String(webserviceLocation.toEncoded()) + "\");";
          code += "const QString messageNamespace = QString::fromLatin1(\"" + mWSDL.definitions().targetNamespace() + "\");";
          code += "d_ptr->m_clientInterface = new KDSoapClientInterface(endPoint, messageNamespace);";
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
              convertClientCall( operation, binding, newClass );
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
  mClasses.sortByDependencies();
  // Then add the service, at the end

  mClasses += bindingClasses;
}

void Converter::clientAddOneArgument( KODE::Function& callFunc, const Part& part, KODE::Class &newClass )
{
    const QString lowerName = lowerlize( part.name() );
    const QString argType = mTypeMap.localInputType( part.type(), part.element() );
    //qDebug() << "localInputType" << part.type().qname() << part.element().qname() << "->" << argType;
    if ( argType != "void" ) {
        callFunc.addArgument( argType + ' ' + mNameMapper.escape( lowerName ) );
    }
    newClass.addHeaderIncludes( mTypeMap.headerIncludes( part.type() ) );
}

void Converter::clientAddArguments( KODE::Function& callFunc, const Message& message, KODE::Class &newClass )
{
    const Part::List parts = message.parts();
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

void Converter::clientAddMessageArgument( KODE::Code& code, const SoapBinding::Style& bindingStyle, const Part& part )
{
    const QString lowerName = lowerlize( part.name() );
    QString argType = mTypeMap.localType( part.type(), part.element() );
    const bool builtin = mTypeMap.isBuiltinType( part.type(), part.element() );
    const bool isComplex = mTypeMap.isComplexType( part.type(), part.element() );
    if ( argType != "void" ) {
        if ( bindingStyle == SoapBinding::DocumentStyle ) {
            // In document style, the "part" is directly added as arguments
            // See http://www.ibm.com/developerworks/webservices/library/ws-whichwsdl/
            if ( builtin )
                qDebug() << "ERROR: Got a builtin/basic type in document style:" << part.type() << part.element() << "Didn't think this could happen.";
            if ( isComplex ) {
                code += "message.arguments() += " + lowerName + ".serialize(QString()).childValues();";
            } else {
                code += "message.setValue(" + lowerName + ".serialize());";
            }
        } else {
            //qDebug() << "caller:" << part.name() << "type=" << part.type() << "element=" << part.element() << "argType=" << argType << "builtin=" << builtin;
            code.addBlock( appendElementArg( part.type(), part.element(), part.name(), lowerName, "message.childValues()" ) );
#if 0 // old
            const QString partNameStr = "QLatin1String(\"" + part.name() + "\")";
            const QString valueStr = builtin ? lowerName : (lowerName + ".serialize()");
            code += "message.addArgument(" + partNameStr + ", " + valueStr
                    + ", QString::fromLatin1(\"" + type.nameSpace() + "\"), QString::fromLatin1(\"" + type.localName() + "\"));";
#endif
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

    const Part::List parts = message.parts();
    Q_FOREACH( const Part& part, parts ) {
        clientAddMessageArgument( code, soapStyle(binding), part );
    }
}

// Generate synchronous call
void Converter::convertClientCall( const Operation &operation, const Binding &binding, KODE::Class &newClass )
{
  const QString methodName = lowerlize( operation.name() );
  KODE::Function callFunc( mNameMapper.escape( methodName ), "void", KODE::Function::Public );
  callFunc.setDocs(QString("Blocking call to %1.\nNot recommended in a GUI thread.").arg(operation.name()));
  const Message inputMessage = mWSDL.findMessage( operation.input().message() );
  const Message outputMessage = mWSDL.findMessage( operation.output().message() );
  clientAddArguments( callFunc, inputMessage, newClass );
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
  const Part::List outParts = outputMessage.parts();
  if (outParts.count() > 1) {
      qWarning().nospace() << "ERROR: " << methodName << ": complex return types are not implemented yet in sync calls; use an async call";
      // the async code (convertClientOutputMessage) actually supports it, since it can emit multiple values in the signal
  }
  QString retType;
  bool isBuiltin = false;
  bool isComplex = false;
  Q_FOREACH( const Part& outPart, outParts ) {
      //const QString lowerName = lowerlize( outPart.name() );

      retType = mTypeMap.localType( outPart.type(), outPart.element() );
      isBuiltin = mTypeMap.isBuiltinType( outPart.type(), outPart.element() );
      isComplex = mTypeMap.isComplexType( outPart.type(), outPart.element() );
      //qDebug() << retType << "isComplex=" << isComplex;

      callFunc.setReturnType( retType );
      break; // only one...
  }

  code += "if (d_ptr->m_lastReply.isFault())";
  code.indent();
  code += "return " + retType + "();"; // default-constructed value
  code.unindent();

  // WARNING: if you change the logic below, also adapt the result parsing for async calls

  if ( retType != "void" )
  {

      if ( isComplex && soapStyle(binding) == SoapBinding::DocumentStyle /*no wrapper*/ ) {
          code += retType + " ret;"; // local var
          code += "ret.deserialize(d_ptr->m_lastReply.arguments());";
          code += "return ret;";
      } else { // RPC style (adds a wrapper), or simple value
          if ( isBuiltin ) {
              code += QString("return d_ptr->m_lastReply.arguments().first().value().value<%1>();").arg(retType);
          } else {
              code += retType + " ret;"; // local var
              if ( isComplex )
                  code += "ret.deserialize(d_ptr->m_lastReply.arguments().first().childValues());";
              else
                  code += "ret.deserialize(d_ptr->m_lastReply.arguments().first().value());";
              code += "return ret;";
          }
      }

  }

  callFunc.setBody( code );

  newClass.addFunction( callFunc );
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
  clientAddArguments( asyncFunc, message, newClass );
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
  slotCode += "emit " + errorSignal.name() + "(reply);";
  slotCode.unindent();
  slotCode += "} else {";
  slotCode.indent();
  slotCode += "const KDSoapValueList args = reply.arguments();";

  const Message message = mWSDL.findMessage( operation.output().message() );

  QStringList partNames;
  const Part::List parts = message.parts();
  Q_FOREACH( const Part& part, parts ) {
    const QString partType = mTypeMap.localType( part.type(), part.element() );
    Q_ASSERT(!partType.isEmpty());
    const bool isBuiltin = mTypeMap.isBuiltinType( part.type(), part.element() );
    const bool isComplex = mTypeMap.isComplexType( part.type(), part.element() );

    if ( partType == "void" )
        continue;

    QString lowerName = mNameMapper.escape( lowerlize( part.name() ) );
    doneSignal.addArgument( mTypeMap.localInputType( part.type(), part.element() ) + ' ' + lowerName );

    if ( isComplex && soapStyle(binding) == SoapBinding::DocumentStyle /*no wrapper*/ ) {
        slotCode += partType + " ret;"; // local var
        slotCode += "ret.deserialize(args);";
        partNames << "ret";
    } else { // RPC style (adds a wrapper) or simple value
        QString value = "args.child(QLatin1String(\"" + part.name() + "\"))";
        if ( isBuiltin ) {
            partNames << value + ".value().value<" + partType + ">()";
        } else {
            slotCode += partType + " ret;"; // local var. TODO ret1/ret2 etc. if more than one.
            if (isComplex)
                value += ".childValues()";
            else
                value += ".value()";
            slotCode += "ret.deserialize(" + value + ");";
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
        KODE::Function headerSetter( methodName, "void", KODE::Function::Public );
        headerSetter.setDocs(QString("Sets the header '%1', for all subsequent method calls.").arg(partName));
        clientAddOneArgument( headerSetter, part, newClass );
        KODE::Code code;
        code += "KDSoapMessage message;";
        if ( header.use() == SoapBinding::EncodedUse )
            code += "message.setUse(KDSoapMessage::EncodedUse);";
        else
            code += "message.setUse(KDSoapMessage::LiteralUse);";
        clientAddMessageArgument( code, SoapBinding::RPCStyle, part );
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
