/*
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

#include "settings.h"

#include "converter.h"
#include <QDebug>

using namespace KWSDL;

QString upperlize( const QString &str )
{
  return str[ 0 ].toUpper() + str.mid( 1 );
}

QString lowerlize( const QString &str )
{
  return str[ 0 ].toLower() + str.mid( 1 );
}


Converter::Converter()
{    
  mQObject = KODE::Class( "QObject" );
}

void Converter::setWSDL( const WSDL &wsdl )
{
  mWSDL = wsdl;

  // merge namespaces from wsdl and schema
  QStringList namespaces = wsdl.definitions().type().types().namespaces();
  namespaces.append( "http://schemas.xmlsoap.org/soap/encoding/" );
  const QStringList wsdlNamespaces = wsdl.namespaceManager().uris();
  for ( int i = 0; i < wsdlNamespaces.count(); ++i ) {
    if ( !namespaces.contains( wsdlNamespaces[ i ] ) )
      namespaces.append( wsdlNamespaces[ i ] );
  }

  // Keep the prefixes from the wsdl parsing, they are more meaningful than ns1 :)
  mNSManager = wsdl.namespaceManager();

  // overwrite some default prefixes
  mNSManager.setPrefix( "soapenc", "http://schemas.xmlsoap.org/soap/encoding/" );
  mNSManager.setPrefix( "http", "http://schemas.xmlsoap.org/wsdl/http/" );
  mNSManager.setPrefix( "soap", "http://schemas.xmlsoap.org/wsdl/soap/" );
  mNSManager.setPrefix( "xsd", "http://www.w3.org/2001/XMLSchema" );
  mNSManager.setPrefix( "xsi", "http://www.w3.org/2001/XMLSchema-instance" );

  // overwrite with prefixes from settings
  Settings::NSMapping mapping = Settings::self()->namespaceMapping();
  Settings::NSMapping::Iterator it;
  for ( it = mapping.begin(); it != mapping.end(); ++it )
    mNSManager.setPrefix( it.value(), it.key() );

//  mNSManager.dump();

  mTypeMap.setNSManager( &mNSManager );

  cleanupUnusedTypes();

  // set the xsd types
  mTypeMap.addSchemaTypes( mWSDL.definitions().type().types() );

  if (qgetenv("KDSOAP_TYPE_DEBUG").toInt())
    mTypeMap.dump();
}

void Converter::cleanupUnusedTypes()
{
    // Keep only the portTypes, messages, and types that are actually used, no point in generating unused classes.

    Definitions definitions = mWSDL.definitions();
    Type type = definitions.type();
    XSD::Types types = type.types();

    const bool printDebug = qgetenv("KDSOAP_TYPE_DEBUG").toInt();
    if (printDebug) {
        qDebug() << "Before cleanup:";
        qDebug() << definitions.messages().count() << "messages";
        qDebug() << types.complexTypes().count() << "complex types";
        qDebug() << types.simpleTypes().count() << "simple types";
        qDebug() << types.elements().count() << "elements";

        //Q_FOREACH(const XSD::Element& elem, types.elements()) {
        //    qDebug() << "element:" << elem.qualifiedName();
        //}
    }

    QSet<QName> usedMessageNames;
    //QSet<QName> portTypeNames;
    Q_FOREACH( const Port& port, definitions.service().ports() ) {
        Binding binding = mWSDL.findBinding( port.bindingName() );
        //portTypeNames.insert( binding.portTypeName() );
        PortType portType = mWSDL.findPortType( binding.portTypeName() );
        const Operation::List operations = portType.operations();
        //qDebug() << "portType" << portType.name() << operations.count() << "operations";
        Q_FOREACH( const Operation& operation, operations ) {
            //qDebug() << "  operation" << operation.operationType();
            switch(operation.operationType()) {
            case Operation::OneWayOperation:
                usedMessageNames.insert(operation.input().message());
                break;
            case Operation::RequestResponseOperation:
            case Operation::SolicitResponseOperation:
                usedMessageNames.insert(operation.input().message());
                usedMessageNames.insert(operation.output().message());
                break;
            case Operation::NotificationOperation:
                usedMessageNames.insert(operation.output().message());
                break;
            };
        }
    }

    // Keep only the messages in usedMessageNames
    QSet<QName> usedTypes;
    QSet<QString> usedTypesStrings; // for debug
    QSet<QName> usedElementNames;
    Message::List newMessages;
    Q_FOREACH(const QName& messageName, usedMessageNames.toList() /*slow!*/) {
        //qDebug() << messageName;
        Message message = mWSDL.findMessage(messageName);
        newMessages.append(message);
        Q_FOREACH(const Part& part, message.parts()) {
            if (!part.type().isEmpty()) {
                usedTypes.insert(part.type());
                usedTypesStrings.insert(part.type().qname());
            } else {
                const QName elemName = part.element();
                XSD::Element element = mWSDL.findElement(elemName);
                usedElementNames.insert(element.qualifiedName());
                usedTypes.insert(element.type());
                usedTypesStrings.insert(element.type().qname());
            }
        }
    }

    //qDebug() << "usedTypes:" << usedTypesStrings.toList();

    // keep only the types used in these messages
    XSD::ComplexType::List usedComplexTypes;
    XSD::SimpleType::List usedSimpleTypes;
    QSet<QName> allUsedTypes = usedTypes;
    do {
        QSet<QName> alsoUsedTypes;
        Q_FOREACH(const QName& typeName, usedTypes.toList() /*slow!*/) {
            if (typeName.isEmpty())
                continue;
            if (mTypeMap.isBuiltinType(typeName))
                continue;
            //qDebug() << typeName;
            XSD::ComplexType complexType = types.complexType(typeName);
            if (!complexType.name().isEmpty()) { // found it as a complex type
                usedComplexTypes.append(complexType);

                Q_FOREACH(const XSD::Element& element, complexType.elements()) {
                    if (!allUsedTypes.contains(element.type()) && !alsoUsedTypes.contains(element.type())) {
                        alsoUsedTypes.insert(element.type());
                        allUsedTypes.insert(element.type());
                    }
                }

            } else {
                XSD::SimpleType simpleType = types.simpleType(typeName);
                if (!simpleType.name().isEmpty()) {
                    usedSimpleTypes.append(simpleType);
                    if (!allUsedTypes.contains(simpleType.baseTypeName()) && !alsoUsedTypes.contains(simpleType.baseTypeName())) {
                        alsoUsedTypes.insert(simpleType.baseTypeName());
                        allUsedTypes.insert(simpleType.baseTypeName());
                    }
                } // we rely on the warning in simpleType if not found.
            }
        }
        usedTypes = alsoUsedTypes;
    } while (!usedTypes.isEmpty());

    XSD::Element::List usedElements;
    QSetIterator<QName> elemIt(usedElementNames);
    while (elemIt.hasNext()) {
        const QName name = elemIt.next();
        XSD::Element element = mWSDL.findElement(name);
        if (element.name().isEmpty())
            qDebug() << name << "not found";
        else
            usedElements.append(element);
    }
    definitions.setMessages(newMessages);
    types.setComplexTypes(usedComplexTypes);
    types.setSimpleTypes(usedSimpleTypes);
    types.setElements(usedElements);
    type.setTypes(types);
    definitions.setType(type);

    mWSDL.setDefinitions(definitions);

    if (printDebug) {
        qDebug() << "After cleanup:";
        qDebug() << definitions.messages().count() << "messages";
        qDebug() << types.complexTypes().count() << "complex types";
        qDebug() << types.simpleTypes().count() << "simple types";
        qDebug() << types.elements().count() << "elements";
    }
}

KODE::Class::List Converter::classes() const
{
  return mClasses;
}

void Converter::convert()
{
  createSoapUtils();

  convertTypes();

//  mNSManager.dump();

  // TODO: allow server service
  convertClientService();
}

void Converter::convertTypes()
{
  const XSD::Types types = mWSDL.definitions().type().types();

  XSD::SimpleType::List simpleTypes = types.simpleTypes();
  qDebug() << "Converting" << simpleTypes.count() << "simple types";
  for ( int i = 0; i < simpleTypes.count(); ++i )
    convertSimpleType( &(simpleTypes[ i ]), simpleTypes );

  XSD::ComplexType::List complexTypes = types.complexTypes();
  qDebug() << "Converting" << complexTypes.count() << "complex types";
  for ( int i = 0; i < complexTypes.count(); ++i )
    convertComplexType( &(complexTypes[ i ]) );

  XSD::Attribute::List attributes = types.attributes();
  qDebug() << "Converting" << attributes.count() << "attributes";
  for ( int i = 0; i < attributes.count(); ++i )
    convertAttribute( &(attributes[ i ]) );

  //XSD::Element::List elements = types.elements();
  //qDebug() << "Converting" << elements.count() << "elements";
  //for ( int i = 0; i < elements.count(); ++i )
  //  convertElement( &(elements[ i ]) );
}
