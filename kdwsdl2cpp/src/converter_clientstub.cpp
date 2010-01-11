/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

using namespace KWSDL;

void Converter::convertClientService()
{
  const Service service = mWSDL.definitions().service();

  KODE::Class newClass( service.name() );
  newClass.addBaseClass( mQObject );
  newClass.addHeaderInclude( "QObject" );
  newClass.addHeaderInclude( "QString" );

  KODE::Function ctor( service.name() );
  KODE::Function dtor( '~' + service.name() );
  KODE::Code ctorCode, dtorCode;

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
    }

    PortType portType = mWSDL.findPortType( binding.portTypeName() );
    const Operation::List operations = portType.operations();
    Operation::List::ConstIterator opIt;
    for ( opIt = operations.begin(); opIt != operations.end(); ++opIt ) {
      if ( (*opIt).operationType() == Operation::OneWayOperation ) {
        convertClientInputMessage( *opIt, (*opIt).input(), binding, newClass );
      } else if ( (*opIt).operationType() == Operation::RequestResponseOperation ) {
        convertClientInputMessage( *opIt, (*opIt).input(), binding, newClass );
        convertClientOutputMessage( *opIt, (*opIt).output(), binding, newClass );
        // TODO fault
      } else if ( (*opIt).operationType() == Operation::SolicitResponseOperation ) {
        convertClientOutputMessage( *opIt, (*opIt).output(), binding, newClass );
        convertClientInputMessage( *opIt, (*opIt).input(), binding, newClass );
        // TODO fault
      } else if ( (*opIt).operationType() == Operation::NotificationOperation ) {
        convertClientOutputMessage( *opIt, (*opIt).output(), binding, newClass );
      }

      QString operationName = lowerlize( (*opIt).name() );

      KODE::MemberVariable transport( operationName + "Transport", "Transport*" );
      newClass.addMemberVariable( transport );

      ctorCode += transport.name() + " = new Transport( \"" + webserviceLocation.toString() + "\" );";
      ctorCode += "connect( " + transport.name() + ", SIGNAL( result( const QString& ) ),";
      ctorCode.indent();
      ctorCode += "this, SLOT( " + operationName + "Slot( const QString& ) ) );";
      ctorCode.unindent();
      ctorCode += "connect( " + transport.name() + ", SIGNAL( error( const QString& ) ),";
      ctorCode.indent();
      ctorCode += "this, SLOT( " + operationName + "ErrorSlot( const QString& ) ) );";
      ctorCode.unindent();


      dtorCode += "delete " + transport.name() + ';';
      dtorCode += transport.name() + " = 0;";
    }
  }

  ctor.setBody( ctorCode );
  newClass.addFunction( ctor );

  dtor.setBody( dtorCode );
  newClass.addFunction( dtor );

  mClasses.append( newClass );
}

void Converter::convertClientInputMessage( const Operation &operation, const Param &param,
                                           const Binding &binding, KODE::Class &newClass )
{
  // call
  QString operationName = lowerlize( operation.name() );
  KODE::Function callFunc( mNameMapper.escape( operationName ), "void", KODE::Function::Public );

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

  const Message message = mWSDL.findMessage( param.message() );

  const Part::List parts = message.parts();
  Part::List::ConstIterator it;
  for ( it = parts.begin(); it != parts.end(); ++it ) {
    QString lowerName = lowerlize( (*it).name() );

    QName type = (*it).type();
    if ( !type.isEmpty() ) {
      callFunc.addArgument( mTypeMap.localType( type ) + " *" + mNameMapper.escape( lowerName ) );
    } else {
      callFunc.addArgument( mTypeMap.localTypeForElement( (*it).element() ) + " *" + mNameMapper.escape( lowerName ) );
    }
  }

  KODE::Code code;
  code += "QDomDocument doc;";
  code += "doc.appendChild( doc.createProcessingInstruction( \"xml\", \"version=\\\"1.0\\\" encoding=\\\"UTF-8\\\"\" ) );";
  code += "QDomElement env = doc.createElement( \"SOAP-ENV:Envelope\" );";
  code += "env.setAttribute( \"xmlns:SOAP-ENV\", \"http://schemas.xmlsoap.org/soap/envelope/\" );";

  const QStringList prefixes = mNSManager.prefixes();
  for ( int i = 0; i < prefixes.count(); ++i )
    code += "env.setAttribute( \"xmlns:" + prefixes[ i ] + "\", \"" + mNSManager.uri( prefixes[ i ] ) + "\" );";

  code += "doc.appendChild( env );";
  if ( !soapHeaderType.isEmpty() ) {
    code += "QDomElement header = doc.createElement( \"SOAP-ENV:Header\" );";
    code += "env.appendChild( header );";
    code += "Serializer::marshal( doc, header, \"" + soapHeaderName + "\", _header, false );";
    code += "delete _header;";
  }
  code += "QDomElement body = doc.createElement( \"SOAP-ENV:Body\" );";
  code += "env.appendChild( body );";
  code.newLine();

  QString parentNode = "body";

  // if SOAP style is RPC, we have to add an extra wrapper element
  if ( soapStyle == SoapBinding::RPCStyle ) {
    code += "QDomElement wrapper = doc.createElement( \"" + mNSManager.fullName( soapBody.nameSpace(), operation.name() ) + "\" );";
    // encodingStyle can be empty or not existent, we have to differ here
    if ( !soapBody.encodingStyle().isNull() )
      code += "wrapper.setAttribute( \"SOAP-ENV:encodingStyle\", \"" + soapBody.encodingStyle() + "\" );";
    code += "body.appendChild( wrapper );";

    parentNode = "wrapper";
  }

  for ( it = parts.begin(); it != parts.end(); ++it ) {
    QString lowerName = lowerlize( (*it).name() );

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
  }

  QString header = "QString()";
  if ( binding.type() == Binding::SOAPBinding ) {
    const SoapBinding soapBinding( binding.soapBinding() );
    const SoapBinding::Operation::Map map = soapBinding.operations();
    const SoapBinding::Operation op = map[ operation.name() ];
    header = "\"\\\"" + op.action() + "\\\"\"";
  }

  code += "qDebug( \"%s\", qPrintable( doc.toString() ) );";
  KODE::MemberVariable transport( operationName + "Transport", "Transport*" );
  code += transport.name() + "->query( doc.toString(), " + header + " );";
  callFunc.setBody( code );

  newClass.addFunction( callFunc );
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
  KODE::Function respSignal( operationName + "Response", "void", KODE::Function::Signal );

  respSignal.setDocs( "This signal is emitted whenever the call to " + operationName+ "() succeeded." );

  // If one output message is used by two input messages, don't define
  // it twice.
  if ( newClass.hasFunction( respSignal.name() ) )
    return;

  KODE::Code code;

  const Message message = mWSDL.findMessage( param.message() );

  const Part::List parts = message.parts();
  for ( int i = 0; i < parts.count(); ++i ) {
    QString partType;
    QName type = parts[ i ].type();
    if ( !type.isEmpty() ) {
      partType = mTypeMap.localType( type );
    } else {
      partType = mTypeMap.localTypeForElement( parts[ i ].element() );
    }

    QString lowerName = mNameMapper.escape( lowerlize( parts[ i ].name() ) );

    respSignal.addArgument( partType + " *" + lowerName );

    code += partType + " *" + lowerName + " = 0;";
  }
  code.newLine();

  newClass.addFunction( respSignal );

  // error signal
  KODE::Function errorSignal( operationName + "Error", "void", KODE::Function::Signal );
  errorSignal.addArgument( "SoapFault *error" );
  errorSignal.setDocs( "This signal is emitted whenever the call to " + operationName+ "() failed." );

  newClass.addFunction( errorSignal );

  // result slot
  KODE::Function respSlot( operationName + "Slot", "void", KODE::Function::Slot | KODE::Function::Private );
  respSlot.addArgument( "const QString &xml" );

  code += "QDomDocument doc;";
  code += "QString errorMsg;";
  code += "int column, row;";
  code.newLine();
  code += "qDebug( \"%s\", qPrintable( xml ) );";
  code.newLine();
  code += "if ( !doc.setContent( xml, true, &errorMsg, &row, &column ) ) {";
  code.indent();
  code += "qDebug( \"Unable to parse xml: %s (%d:%d)\", qPrintable( errorMsg ), row, column );";
  code += "return;";
  code.unindent();
  code += '}';
  code.newLine();
  code += "QDomElement envelope = doc.documentElement();";
  code += "QDomElement body = envelope.firstChild().toElement();";

  if ( soapStyle == SoapBinding::RPCStyle ) {
    code += "QDomElement method = body.firstChild().toElement();";
    code += "if ( method.tagName() == \"Fault\" ) {";
    code.indent();
    code += "SoapFault *fault = new SoapFault;";
    code += "Serializer::demarshal( method, fault );";
    code += "emit " + errorSignal.name() + "( fault );";
    code.unindent();
    code += "} else {";
    code.indent();
    code += "QDomNode node = method.firstChild();";
    code += "while ( !node.isNull() ) {";
    code.indent();
    code += "QDomElement element = node.toElement();";
    code += "if ( !element.isNull() ) {";
    code.indent();

    QStringList partNames;
    for ( int i = 0; i < parts.count(); ++i ) {
      QString partType;

      QName type = parts[ i ].type();
      if ( !type.isEmpty() ) {
        partType = mTypeMap.localType( type );
      } else {
        partType = mTypeMap.localTypeForElement( parts[ i ].element() );
      }

      QString lowerName = mNameMapper.escape( lowerlize( parts[ i ].name() ) );
      partNames << lowerName;

      code += "if ( element.tagName() == \"" + parts[ i ].name() + "\" ) {";
      code.indent();
      code += lowerName + " = new " + partType + "();";
      code += "Serializer::demarshal( method.firstChild().toElement(), " + lowerName + " );";
      code.unindent();
      code += '}';
    }
    code.unindent();
    code += '}';
    code.newLine();
    code += "node = node.nextSibling();";
    code.unindent();
    code += '}';
    code.newLine();
    code += "emit " + respSignal.name() + "( " + partNames.join( "," ) + " );";
    code.unindent();
    code += '}';
  } else { // soapStyle == SoapBinding::DocumentStyle
    code += "QDomElement method = body.firstChild().toElement();";
    code += "if ( method.tagName() == \"Fault\" ) {";
    code.indent();
    code += "SoapFault *fault = new SoapFault;";
    code += "Serializer::demarshal( method, fault );";
    code += "emit " + errorSignal.name() + "( fault );";
    code.unindent();
    code += "} else {";
    code.indent();
    code += "QDomNode node = body.firstChild();";
    code += "while ( !node.isNull() ) {";
    code.indent();
    code += "QDomElement element = node.toElement();";
    code += "if ( !element.isNull() ) {";
    code.indent();

    code += "if ( element.tagName() == \"Fault\" ) {";
    code.indent();
    code += "SoapFault *fault = new SoapFault;";
    code += "Serializer::demarshal( element, fault );";
    code += "emit " + errorSignal.name() + "( fault );";
    code += "return;";
    code.unindent();
    code += '}';

    QStringList partNames;
    for ( int i = 0; i < parts.count(); ++i ) {
      QString partType;

      QName type = parts[ i ].type();
      if ( !type.isEmpty() ) {
        partType = mTypeMap.localType( type );
      } else {
        partType = mTypeMap.localTypeForElement( parts[ i ].element() );
      }

      QString lowerName = mNameMapper.escape( lowerlize( parts[ i ].name() ) );
      partNames << lowerName;

      code += "if ( element.tagName() == \"" + parts[ i ].name() + "\" ) {";
      code.indent();
      code += lowerName + " = new " + partType + "();";
      code += "Serializer::demarshal( method.firstChild().toElement(), " + lowerName + " );";
      code.unindent();
      code += '}';
    }
    code.unindent();
    code += '}';
    code.newLine();
    code += "node = node.nextSibling();";
    code.unindent();
    code += '}';
    code.newLine();
    code += "emit " + respSignal.name() + "( " + partNames.join( "," ) + " );";
    code.unindent();
    code += '}';
  }

  respSlot.setBody( code );

  newClass.addFunction( respSlot );

  // error slot
  KODE::Function errorSlot( operationName + "ErrorSlot", "void", KODE::Function::Slot | KODE::Function::Private );
  errorSlot.addArgument( "const QString &msg" );
  code.clear();

  code += "SoapFault *fault = new SoapFault;";
  code += "fault->setCode( \"Connection Error\" );";
  code += "fault->setDescription( msg );";
  code += "emit " + errorSignal.name() + "( fault );";

  errorSlot.setBody( code );

  newClass.addFunction( errorSlot );
}
