/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2010 David Faure <dfaure@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "converter.h"
#include <QDebug>

using namespace KWSDL;

void Converter::convertClientService()
{
  const Service service = mWSDL.definitions().service();

  Q_ASSERT(!service.name().isEmpty());

  KODE::Class newClass( service.name() );
  newClass.setUseDPointer(true);
  newClass.addBaseClass( mQObject );
  newClass.setDocs(service.documentation());

  // Files included in the header
  newClass.addHeaderInclude( "QObject" );
  newClass.addHeaderInclude( "QString" );

  // Files included in the impl, with optional forward-declarations in the header
  newClass.addInclude("KDSoapMessage.h", "KDSoapMessage");
  newClass.addInclude("KDSoapClientInterface.h", "KDSoapClientInterface");
  newClass.addInclude("KDSoapPendingCallWatcher.h", "KDSoapPendingCallWatcher");

  // Variables (which will go into the d pointer)
  KODE::MemberVariable clientInterfaceVar("m_clientInterface", "KDSoapClientInterface*");
  clientInterfaceVar.setInitializer("NULL");
  newClass.addMemberVariable(clientInterfaceVar);

  KODE::MemberVariable lastReply("m_lastReply", "KDSoapMessage");
  newClass.addMemberVariable(lastReply);

  // Ctor and dtor
  {
      KODE::Function ctor( service.name() ); // TODO add QObject* parent = 0 argument
      ctor.addArgument("QObject* parent", "0");
      ctor.addInitializer("QObject(parent)");
      KODE::Function dtor( '~' + service.name() );
      KODE::Code ctorCode, dtorCode;

      ctor.setBody( ctorCode );
      newClass.addFunction( ctor );

      dtorCode += "delete d->m_clientInterface;";

      dtor.setBody( dtorCode );
      newClass.addFunction( dtor );
  }
  // lastError() method
  {
      KODE::Function lastError("lastError", "QString");
      KODE::Code code;
      code += "if (d->m_lastReply.isFault())";
      code.indent();
      code += "return d->m_lastReply.faultAsString();";
      code.unindent();
      code += "return QString();";
      lastError.setBody(code);
      lastError.setDocs("Return the error from the last blocking call.\nEmpty if no error.");
      newClass.addFunction(lastError);
  }

  const Port::List servicePorts = service.ports();
  Port::List::ConstIterator it;
  for ( it = servicePorts.begin(); it != servicePorts.end(); ++it ) {
    Binding binding = mWSDL.findBinding( (*it).bindingName() );

    // TODO: more flexible binding handling
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

    // TODO: what if there are multiple soap ports?
    // clientInterface() private method
    {
        KODE::Function clientInterface("clientInterface", "KDSoapClientInterface*", KODE::Function::Private);
        KODE::Code code;
        code += "if (!d->m_clientInterface) {";
        code.indent();
        code += "const QString endPoint = QString::fromLatin1(\"" + QLatin1String(webserviceLocation.toEncoded()) + "\");";
        // TODO? see Parser::targetNamespace() code += "const QString messageNamespace = QString::fromLatin1(\"" +  + "\");";
        code += "d->m_clientInterface = new KDSoapClientInterface(endPoint, QString() /*TODO*/);";
        code.unindent();
        code += "}";
        code += "return d->m_clientInterface;";
        clientInterface.setBody(code);
        newClass.addFunction(clientInterface);
    }

    PortType portType = mWSDL.findPortType( binding.portTypeName() );
    //qDebug() << portType.name();
    const Operation::List operations = portType.operations();
    Operation::List::ConstIterator opIt;
    for ( opIt = operations.begin(); opIt != operations.end(); ++opIt ) {
        Operation::OperationType opType = (*opIt).operationType();
        switch(opType) {
        case Operation::OneWayOperation:
            convertClientInputMessage( *opIt, (*opIt).input(), binding, newClass );
            break;
        case Operation::RequestResponseOperation: // the standard case
            // sync method
            convertClientCall( *opIt, binding, newClass );
            // async method
            convertClientInputMessage( *opIt, (*opIt).input(), binding, newClass );
            convertClientOutputMessage( *opIt, (*opIt).output(), binding, newClass );
            // TODO fault
            break;
        case Operation::SolicitResponseOperation:
            convertClientOutputMessage( *opIt, (*opIt).output(), binding, newClass );
            convertClientInputMessage( *opIt, (*opIt).input(), binding, newClass );
            // TODO fault
            break;
        case Operation::NotificationOperation:
            convertClientOutputMessage( *opIt, (*opIt).output(), binding, newClass );
            break;
        }

#ifdef KDAB_DELETED
      QString operationName = lowerlize( (*opIt).name() );
      ctorCode += transport.name() + " = new Transport( \"" + webserviceLocation.toString() + "\" );";
#endif
    }
  }

  mClasses.append(newClass);
}

// This code was in convertClientInputMessage
#ifdef KDAB_TEMP
{
  // handle soap header
  QString soapHeaderType;
  QString soapHeaderName;
  SoapBinding::Style soapStyle = SoapBinding::RPCStyle;
  SoapBinding::Body soapBody;

  if ( binding.type() == Binding::SOAPBinding ) {
    const SoapBinding soapBinding = binding.soapBinding();
    soapStyle = soapBinding.binding().style();
    const SoapBinding::Operation::Map operations = soapBinding.operations();
    const SoapBinding::Operation soapOperation = operations[ operation.name() ];
    soapBody = soapOperation.input();
    const SoapBinding::Header header = soapOperation.inputHeader();

    if ( !header.message().isEmpty() ) {
      const Message message = mWSDL.findMessage( header.message() );
      const Part::List parts = message.parts();
      for ( int i = 0; i < parts.count(); ++i ) {
        if ( parts[ i ].name() == header.part() ) {
          QName type = parts[ i ].type();
          if ( !type.isEmpty() ) {
            soapHeaderType = mTypeMap.localType( type );
            soapHeaderName = mNSManager.fullName( type.nameSpace(), type.localName() );
          } else {
            QName element = parts[ i ].element();
            soapHeaderType = mTypeMap.localTypeForElement( element );
            soapHeaderName = mNSManager.fullName( element.nameSpace(), element.localName() );
          }

          callFunc.addArgument( soapHeaderType + " *_header" );
          break;
        }
      }
    }
  }
}
#endif

void Converter::clientAddArguments( KODE::Function& callFunc, const Message& message )
{
    const Part::List parts = message.parts();
    Part::List::ConstIterator it;
    for ( it = parts.begin(); it != parts.end(); ++it ) {
        const QString lowerName = lowerlize( (*it).name() );

        QName type = (*it).type();
        if ( !type.isEmpty() ) {
            callFunc.addArgument( mTypeMap.localType( type, true ) + " " + mNameMapper.escape( lowerName ) );
        } else {
            callFunc.addArgument( mTypeMap.localTypeForElement( (*it).element() ) + " " + mNameMapper.escape( lowerName ) );
        }
    }
}

bool Converter::clientAddAction( KODE::Code& code, const Binding &binding, const QString& operationName )
{
    bool hasAction = false;
    if ( binding.type() == Binding::SOAPBinding ) {
        const SoapBinding soapBinding( binding.soapBinding() );
        const SoapBinding::Operation::Map map = soapBinding.operations();
        const SoapBinding::Operation op = map[ operationName ];
        if (!op.action().isEmpty()) {
            code += "const QString action = QString::fromLatin1(\"" + op.action() + "\");";
            hasAction = true;
        }
    }
    return hasAction;
}

void Converter::clientGenerateMessage( KODE::Code& code, const Message& message )
{
    code += "KDSoapMessage message;";
    const Part::List parts = message.parts();
    Part::List::ConstIterator it;
    for ( it = parts.begin(); it != parts.end(); ++it ) {
        const QString lowerName = lowerlize( (*it).name() );

#ifdef KDAB_DELETED // what's the soapStyle stuff? (standard case is RPCStyle)
        QString name, noNamespace;
        if ( soapStyle == SoapBinding::RPCStyle ) {
            name = (*it).name();
            noNamespace = "true";
        } else {
            noNamespace = "false";
            QName type = (*it).type();
            if ( !type.isEmpty() ) {
                name = mNSManager.fullName( type.nameSpace(), type.localName() );
            } else {
                name = mNSManager.fullName( (*it).element().nameSpace(), (*it).element().localName() );
            }
        }
        code += "Serializer::marshal( doc, " + parentNode + ", \"" + name + "\", " + mNameMapper.escape( lowerName ) +
                ", " + noNamespace + " );";
        code += "delete " + mNameMapper.escape( lowerName ) + ';';
#endif
        code += "message.addArgument(QLatin1String(\"" + (*it).name() + "\"), " + lowerName + ");";
    }
}

void Converter::convertClientCall( const Operation &operation, const Binding &binding, KODE::Class &newClass )
{
  QString operationName = lowerlize( operation.name() );
  KODE::Function callFunc( mNameMapper.escape( operationName ), "void", KODE::Function::Public );
  callFunc.setDocs(QString("Blocking call to %1.\nNot recommended in a GUI thread.").arg(operation.name()));
  const Message inputMessage = mWSDL.findMessage( operation.input().message() );
  const Message outputMessage = mWSDL.findMessage( operation.output().message() );
  clientAddArguments( callFunc, inputMessage );
  KODE::Code code;
  const bool hasAction = clientAddAction( code, binding, operation.name() );
  clientGenerateMessage( code, inputMessage );
  QString callLine = "d->m_lastReply = clientInterface()->call(QLatin1String(\"" + operationName + "\"), message";
  if (hasAction) {
      callLine += ", action";
  }
  callLine += ");";
  code += callLine;

  code += "if (d->m_lastReply.isFault())";
  code.indent();
  code += "return QString();";
  code.unindent();

  // Return value(s) :
  const Part::List outParts = outputMessage.parts();
  if (outParts.count() > 1)
      qWarning().nospace() << "ERROR: " << operationName << ": complex return types are not implemented yet";
  QString retType;
  Part::List::ConstIterator it;
  for ( it = outParts.begin(); it != outParts.end(); ++it ) {
      //const QString lowerName = lowerlize( (*it).name() );
      QName type = (*it).type();
      if ( !type.isEmpty() ) {
          retType = mTypeMap.localType( type );
          callFunc.setReturnType( retType );
      }
  }

  code += QString("return d->m_lastReply.arguments().first().value().value<%1>();").arg(retType);

  callFunc.setBody( code );

  newClass.addFunction( callFunc );
}

void Converter::convertClientInputMessage( const Operation &operation, const Param &param,
                                           const Binding &binding, KODE::Class &newClass )
{
  QString operationName = operation.name();
  KODE::Function asyncFunc( "async" + upperlize( operationName ), "void", KODE::Function::Public );
  asyncFunc.setDocs(QString("Asynchronous call to %1.\n"
                            "Remember to connect to %2 and %3.")
                    .arg(operation.name())
                    .arg(lowerlize(operationName) + "Done")
                    .arg(lowerlize(operationName) + "Error"));
  const Message message = mWSDL.findMessage( param.message() );
  clientAddArguments(asyncFunc, message);
  KODE::Code code;
  const bool hasAction = clientAddAction( code, binding, operation.name() );
  clientGenerateMessage( code, message );

#ifdef KDAB_TEMP
  const QStringList prefixes = mNSManager.prefixes();
  for ( int i = 0; i < prefixes.count(); ++i )
    code += "env.setAttribute( \"xmlns:" + prefixes[ i ] + "\", \"" + mNSManager.uri( prefixes[ i ] ) + "\" );";

  // if SOAP style is RPC, we have to add an extra wrapper element
  if ( soapStyle == SoapBinding::RPCStyle ) {
    code += "QDomElement wrapper = doc.createElement( \"" + mNSManager.fullName( soapBody.nameSpace(), operation.name() ) + "\" );";
    // encodingStyle can be empty or not existent, we have to differ here
    if ( !soapBody.encodingStyle().isNull() )
      code += "wrapper.setAttribute( \"SOAP-ENV:encodingStyle\", \"" + soapBody.encodingStyle() + "\" );";
    code += "body.appendChild( wrapper );";

    parentNode = "wrapper";
  }
#endif

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

void Converter::convertClientOutputMessage( const Operation &operation, const Param &param,
                                            const Binding &binding, KODE::Class &newClass )
{
  SoapBinding::Style soapStyle = SoapBinding::RPCStyle;
  if ( binding.type() == Binding::SOAPBinding ) {
    const SoapBinding soapBinding = binding.soapBinding();
    soapStyle = soapBinding.binding().style();
  }

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

  const Message message = mWSDL.findMessage( param.message() );

  QStringList partNames;
  const Part::List parts = message.parts();
  for ( int i = 0; i < parts.count(); ++i ) {
    QString partType, signaturePartType;
    QName type = parts[ i ].type();
    if ( !type.isEmpty() ) {
      signaturePartType = mTypeMap.localType( type, true ); // e.g. const QString&
      partType = mTypeMap.localType( type ); // e.g. QString
    } else {
      signaturePartType = mTypeMap.localTypeForElement( parts[ i ].element() ); // TODO ,true
      partType = mTypeMap.localTypeForElement( parts[ i ].element() );
    }

    QString lowerName = mNameMapper.escape( lowerlize( parts[ i ].name() ) );

    doneSignal.addArgument( signaturePartType + " " + lowerName );
    partNames << "args.value(\"" + lowerName + "\").value<" + partType + ">()";

#ifdef KDAB_TEMP // initialization of local var
    code += partType + " " + lowerName + " = 0;";
#endif
  }

  newClass.addFunction( doneSignal );
  newClass.addFunction( errorSignal );

  KODE::Code slotCode;
  slotCode += "KDSoapMessage reply = watcher->returnArguments();";
  slotCode += "if (reply.isFault()) {";
  slotCode.indent();
  slotCode += "emit " + errorSignal.name() + "(reply);";
  slotCode.unindent();
  slotCode += "} else {";
  slotCode.indent();
  slotCode += "KDSoapValueList args = reply.arguments();";
  slotCode += "emit " + doneSignal.name() + "( " + partNames.join( "," ) + " );";
  slotCode.unindent();
  slotCode += '}';

  finishedSlot.setBody(slotCode);

  newClass.addFunction(finishedSlot);
}
