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
  mQObject = KODE::Class( QLatin1String("QObject") );
  mKDSoapServerObjectInterface = KODE::Class( QLatin1String("KDSoapServerObjectInterface") );
}

void Converter::setWSDL( const WSDL &wsdl )
{
  mWSDL = wsdl;

  // Keep the prefixes from the wsdl parsing, they are more meaningful than ns1 :)
  mNSManager = wsdl.namespaceManager();

  // overwrite some default prefixes
  mNSManager.setPrefix( QLatin1String( "soapenc"), QLatin1String("http://schemas.xmlsoap.org/soap/encoding/") );
  mNSManager.setPrefix( QLatin1String("http"), QLatin1String("http://schemas.xmlsoap.org/wsdl/http/") );
  mNSManager.setPrefix( QLatin1String("soap"), QLatin1String("http://schemas.xmlsoap.org/wsdl/soap/") );
  mNSManager.setPrefix( QLatin1String("xsd"), QLatin1String("http://www.w3.org/2001/XMLSchema") );
  mNSManager.setPrefix( QLatin1String("xsi"), QLatin1String("http://www.w3.org/2001/XMLSchema-instance") );

  // overwrite with prefixes from settings
  Settings::NSMapping mapping = Settings::self()->namespaceMapping();
  Settings::NSMapping::Iterator it;
  for ( it = mapping.begin(); it != mapping.end(); ++it )
    mNSManager.setPrefix( it.value(), it.key() );

  if (qgetenv("KDSOAP_TYPE_DEBUG").toInt())
      mNSManager.dump();

  mTypeMap.setNSManager( &mNSManager );

  cleanupUnusedTypes();

  // set the xsd types
  mTypeMap.addSchemaTypes( mWSDL.definitions().type().types(), Settings::self()->nameSpace() );

  if (qgetenv("KDSOAP_TYPE_DEBUG").toInt())
    mTypeMap.dump();
}

class TypeCollector
{
public:
    TypeCollector(const QSet<QName>& usedTypes) : m_allUsedTypes(usedTypes) {}

    void collectDependentTypes(const TypeMap& typeMap, const XSD::Types& types)
    {
        QSet<QName> typesToProcess = m_allUsedTypes;
        do {
            m_alsoUsedTypes.clear();
            Q_FOREACH(const QName& typeName, typesToProcess.toList() /*slow!*/) {
                if (typeName.isEmpty())
                    continue;
                if (typeMap.isBuiltinType(typeName))
                    continue;
                //qDebug() << "used type:" << typeName;
                XSD::ComplexType complexType = types.complexType(typeName);
                if (!complexType.name().isEmpty()) { // found it as a complex type
                    usedComplexTypes.append(complexType);

                    addDependency(complexType.baseTypeName());
                    Q_FOREACH(const XSD::Element& element, complexType.elements()) {
                        addDependency(element.type());
                    }
                    Q_FOREACH(const XSD::Attribute& attribute, complexType.attributes()) {
                        addDependency(attribute.type());
                    }
                    addDependency(complexType.arrayType());

                } else {
                    XSD::SimpleType simpleType = types.simpleType(typeName);
                    if (!simpleType.name().isEmpty()) {
                        usedSimpleTypes.append(simpleType);
                        addDependency(simpleType.baseTypeName());
                        if (simpleType.subType() == XSD::SimpleType::TypeList) {
                            addDependency(simpleType.listTypeName());
                        }
                    } // we rely on the warning in simpleType if not found.
                }
            }
            typesToProcess = m_alsoUsedTypes;
        } while (!typesToProcess.isEmpty());
    }

    XSD::ComplexType::List usedComplexTypes;
    XSD::SimpleType::List usedSimpleTypes;
private:
    void addDependency(const QName& type) {
        if (!type.isEmpty() && !m_allUsedTypes.contains(type) && !m_alsoUsedTypes.contains(type)) {
            m_alsoUsedTypes.insert(type);
            m_allUsedTypes.insert(type);
        }
    }

    QSet<QName> m_allUsedTypes; // All already seen types
    QSet<QName> m_alsoUsedTypes; // The list of types to process in the next iteration
};

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
    Q_FOREACH( const Service& service, definitions.services() ) {
        Q_FOREACH( const Port& port, service.ports() ) {
            Binding binding = mWSDL.findBinding( port.bindingName() );
            //portTypeNames.insert( binding.portTypeName() );
            //qDebug() << "binding" << port.bindingName() << binding.name() << "port type" << binding.portTypeName();
            PortType portType = mWSDL.findPortType( binding.portTypeName() );
            const Operation::List operations = portType.operations();
            //qDebug() << "portType" << portType.name() << operations.count() << "operations";
            Q_FOREACH( const Operation& operation, operations ) {
                //qDebug() << "  operation" << operation.operationType() << operation.name();
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
                if ( binding.type() == Binding::SOAPBinding ) {
                    const SoapBinding soapBinding( binding.soapBinding() );
                    const SoapBinding::Operation op = soapBinding.operations().value( operation.name() );
                    Q_FOREACH(const SoapBinding::Header& header, op.inputHeaders()) {
                        usedMessageNames.insert(header.message());
                    }
                }
            }
        }
    }

    // Keep only the messages in usedMessageNames
    QSet<QName> usedTypes;
    QSet<QString> usedTypesStrings; // for debug
    QSet<QName> usedElementNames;
    Message::List newMessages;
    Q_FOREACH(const QName& messageName, usedMessageNames.toList() /*slow!*/) {
        //qDebug() << "used message:" << messageName;
        Message message = mWSDL.findMessage(messageName);
        newMessages.append(message);
        Q_FOREACH(const Part& part, message.parts()) {
            if (!part.type().isEmpty()) {
                usedTypes.insert(part.type());
                usedTypesStrings.insert(part.type().qname());
            } else {
                const QName elemName = part.element();
                XSD::Element element = mWSDL.findElement(elemName);
                if (element.qualifiedName().isEmpty()) {
                    qDebug() << "in message" << messageName << ": element not found:" << elemName.qname();
                } else if (element.type().isEmpty()) {
                    qDebug() << "in message" << messageName << ": element without type:" << elemName.qname();
                } else {
                    usedElementNames.insert(element.qualifiedName());
                    usedTypes.insert(element.type());
                    usedTypesStrings.insert(element.type().qname());
                }
            }
        }
    }

    //qDebug() << "usedTypes:" << usedTypesStrings.toList();

    // keep only the types used in these messages
    TypeCollector collector(usedTypes);
    collector.collectDependentTypes(mTypeMap, types);

    XSD::Element::List usedElements;
    QSetIterator<QName> elemIt(usedElementNames);
    while (elemIt.hasNext()) {
        const QName name = elemIt.next();
        XSD::Element element = mWSDL.findElement(name);

        if (element.type().isEmpty()) {
          qDebug() << "ERROR: Element without type:" << element.qualifiedName() << element.nameSpace() << element.name();
          Q_ASSERT(!element.type().isEmpty());
        }

        if (element.name().isEmpty())
            qDebug() << "cleanupUnusedTypes: element" << name << "not found";
        else
            usedElements.append(element);
    }

    definitions.setMessages(newMessages);
    types.setComplexTypes(collector.usedComplexTypes);
    types.setSimpleTypes(collector.usedSimpleTypes);
    types.setElements(usedElements);
    type.setTypes(types);
    definitions.setType(type);

//// TODO BEGIN: Disambiguate code to be refactored and cleaned up along with cleanupUnusedTypes functionality later
    XSD::Element::List tmpElements = definitions.type().types().elements();
    QSet<QName> tmpElementNames;
    Q_FOREACH( const XSD::Element& element, tmpElements ) {
        tmpElementNames.insert( element.qualifiedName() );
    }
    XSD::SimpleType::List tmpSimpleTypes = definitions.type().types().simpleTypes();
    QSet<QName> tmpSimpleTypeNames;
    Q_FOREACH( const XSD::SimpleType& simpleType, tmpSimpleTypes ) {
        tmpSimpleTypeNames.insert( simpleType.qualifiedName() );
    }
    XSD::ComplexType::List tmpComplexTypes = definitions.type().types().complexTypes();
    QSet<QName> tmpComplexTypeNames;
    Q_FOREACH( const XSD::ComplexType& complexType, tmpComplexTypes ) {
        tmpComplexTypeNames.insert( complexType.qualifiedName() );
    }
    Message::List tmpMessages = definitions.messages();

    QSet<QName> conflictingTypeSet = tmpSimpleTypeNames.intersect( tmpComplexTypeNames );
    if ( conflictingTypeSet.count() > 0 ) {
        QString msg;
        msg.append("\n");
        Q_FOREACH( const QName& name, conflictingTypeSet ) {
            msg.append( "Prefix: " + name.prefix() + " NameSpace: " + name.nameSpace() + " LocalName: " + name.localName() + "\n" );
        }
        qWarning() << "ERROR: There are:" << conflictingTypeSet.count() << "Simple and Complex Types that have the same name present:" << msg;
    }

    // Remove expected conflicts between SimpleType and Element Sets
    QSet<QName> simpleTypeConflictingElementsSubtracted = tmpSimpleTypeNames.subtract( tmpElementNames );
    QSet<QName> elementConflictingSimpleTypesSubtracted = tmpElementNames.subtract( tmpSimpleTypeNames );
    QSet<QName> elementConflictingSimpleTypesSubtractedOriginal = elementConflictingSimpleTypesSubtracted;
    // Convert SimpleType and Elements names to class names by capitalising the first character
    Q_FOREACH( const QName& name, simpleTypeConflictingElementsSubtracted.toList() ) {
        simpleTypeConflictingElementsSubtracted.remove( name );
        QName tmpName( name.nameSpace(), upperlize( name.localName() ) );
        simpleTypeConflictingElementsSubtracted.insert( tmpName );
    }
    Q_FOREACH( const QName& name, elementConflictingSimpleTypesSubtracted.toList() ) {
        elementConflictingSimpleTypesSubtracted.remove( name );
        QName tmpName( name.nameSpace(), upperlize( name.localName() ) );
        elementConflictingSimpleTypesSubtracted.insert( tmpName );
    }
    // Check for unexpected conflicts
    QSet<QName> conflictingSimpleTypeAndElementSet = elementConflictingSimpleTypesSubtracted.intersect( simpleTypeConflictingElementsSubtracted );
    // Resolve unexpected conflictings
    Q_FOREACH( const QName& name, conflictingSimpleTypeAndElementSet.toList() ) {
        Q_FOREACH( const QName& eName, elementConflictingSimpleTypesSubtractedOriginal.toList() ) {
            if ( lowerlize( name.localName() ) == eName.localName() ) {
                // resolve conflicting elements...
                //pre-pend "element" to conflicting Element name
                QString newElementName = QLatin1String( "element" ) + name.localName();
                for ( QList< XSD::Element >::iterator elementIt = tmpElements.begin(); elementIt != tmpElements.end(); ++elementIt ) {
                    if ( (*elementIt).qualifiedName() == eName ) {
                        (*elementIt).setName( newElementName );
                    }
                    if ( (*elementIt).type() == eName ) {
                        if ( (*elementIt).type().prefix().size() ) {
                            QName tmpElementType( (*elementIt).type().nameSpace(), (*elementIt).type().prefix() + QLatin1Char( ':' ) + newElementName );
                            (*elementIt).setType( tmpElementType );
                        }
                        else {
                            QName tmpElementType( (*elementIt).type().nameSpace(), newElementName );
                            (*elementIt).setType( tmpElementType );
                        }
                    }
                }
                for ( QList< XSD::SimpleType >::iterator simpleIt = tmpSimpleTypes.begin(); simpleIt != tmpSimpleTypes.end(); ++simpleIt ) {
                    if ( (*simpleIt).qualifiedName() == eName ) {
                        (*simpleIt).setName( newElementName );
                    }
                }
                // resolve conflicting messages and their constiuate parts...
                for ( QList< Message >::iterator messageIt = tmpMessages.begin(); messageIt != tmpMessages.end(); ++messageIt ) {
                    Part::List parts = (*messageIt).parts();
                    for ( QList< Part >::iterator partIt = parts.begin(); partIt != parts.end(); ++partIt ) {
                        if ( lowerlize( name.localName() ) == (*partIt).element().localName() ) {
                            if ( (*partIt).element().prefix().size() ) {
                                QName tmpPartElement( (*partIt).element().prefix() + QLatin1Char(':') + newElementName );
                                tmpPartElement.setNameSpace( (*partIt).element().nameSpace() );
                                (*partIt).setElement( tmpPartElement );
                            }
                            else {
                                QName tmpPartElement( (*partIt).element().nameSpace(), newElementName );
                                (*partIt).setElement( tmpPartElement );
                            }
                        }
                    }
                    (*messageIt).setParts( parts );
                }
            }
        }
    }

    // Remove expected conflicts between ComplexType and Element Sets
    QSet<QName> complexTypeConflictingElementsSubtracted = tmpComplexTypeNames.subtract( tmpElementNames );
    QSet<QName> elementConflictingComplexTypesSubtracted = tmpElementNames.subtract( tmpComplexTypeNames );
    QSet<QName> elementConflictingComplexTypesSubtractedOriginal = elementConflictingComplexTypesSubtracted;
    // Convert ComplexType and Elements names to class names by capitalising the first character
    Q_FOREACH( const QName& name, complexTypeConflictingElementsSubtracted.toList() ) {
        complexTypeConflictingElementsSubtracted.remove( name );
        QName tmpName( name.nameSpace(), upperlize( name.localName() ) );
        complexTypeConflictingElementsSubtracted.insert( tmpName );
    }
    Q_FOREACH( const QName& name, elementConflictingComplexTypesSubtracted.toList() ) {
        elementConflictingComplexTypesSubtracted.remove( name );
        QName tmpName( name.nameSpace(), upperlize( name.localName() ) );
        elementConflictingComplexTypesSubtracted.insert( tmpName );
    }
    // Check for unexpected conflicts
    QSet<QName> conflictingComplexTypeAndElementSet = elementConflictingComplexTypesSubtracted.intersect( complexTypeConflictingElementsSubtracted );
    // Resolve unexpected conflictings
    Q_FOREACH( const QName& name, conflictingComplexTypeAndElementSet.toList() ) {
        Q_FOREACH( const QName& eName, elementConflictingComplexTypesSubtractedOriginal.toList() ) {
            if ( lowerlize( name.localName() ) == eName.localName() ) {
                // resolve conflicting elements...
                // pre-pend "element" to conflicting Element name
                QString newElementName = QLatin1String( "element" ) + name.localName();
                for ( QList< XSD::Element >::iterator elementIt = tmpElements.begin(); elementIt != tmpElements.end(); ++elementIt ) {
                    if ( (*elementIt).qualifiedName() == eName ) {
                        (*elementIt).setName( newElementName );
                    }
                    if ( (*elementIt).type() == eName ) {
                        if ( (*elementIt).type().prefix().size() )
                        {
                            QName tmpElementType( (*elementIt).type().nameSpace(), (*elementIt).type().prefix() + QLatin1Char( ':' ) + newElementName );
                            (*elementIt).setType( tmpElementType );
                        }
                        else {
                            QName tmpElementType( (*elementIt).type().nameSpace(), newElementName );
                            (*elementIt).setType( tmpElementType );
                        }
                    }
                }
                for ( QList< XSD::ComplexType >::iterator complexIt = tmpComplexTypes.begin(); complexIt != tmpComplexTypes.end(); ++complexIt ) {
                    if ( (*complexIt).qualifiedName() == eName ) {
                        (*complexIt).setName( newElementName );
                        if ( (*complexIt).isArray() ) {
                            if ( (*complexIt).arrayType() == eName ) {
                                if ( (*complexIt).arrayType().prefix().size() ) {
                                    QName tmpArrayType( (*complexIt).arrayType().nameSpace(), (*complexIt).arrayType().prefix() + QLatin1Char( ':' ) + newElementName );
                                    (*complexIt).setArrayType( tmpArrayType );
                                }
                                else {
                                    QName tmpArrayType( (*complexIt).arrayType().nameSpace(), newElementName );
                                    (*complexIt).setArrayType( tmpArrayType );
                                }
                            }
                        }
                    }
                    XSD::Element::List elements = (*complexIt).elements();
                    for ( QList< XSD::Element >::iterator elementIt = elements.begin(); elementIt != elements.end(); ++elementIt ) {
                        if ( (*elementIt).qualifiedName() == eName ) {
                            (*elementIt).setName( newElementName );
                        }
                    }
                    (*complexIt).setElements( elements );
                }
                // resolve conflicting messages and their constiuate parts...
                for ( QList< Message >::iterator messageIt = tmpMessages.begin(); messageIt != tmpMessages.end(); ++messageIt ) {
                    Part::List parts = (*messageIt).parts();
                    for ( QList< Part >::iterator partIt = parts.begin(); partIt != parts.end(); ++partIt ) {
                        if ( lowerlize( name.localName() ) == (*partIt).element().localName() ) {
                            if ( (*partIt).element().prefix().size() ) {
                                QName tmpPartElement( (*partIt).element().prefix() + QLatin1Char(':') + newElementName );
                                tmpPartElement.setNameSpace( (*partIt).element().nameSpace() );
                                (*partIt).setElement( tmpPartElement );
                            }
                            else {
                                QName tmpPartElement( (*partIt).element().nameSpace(), newElementName );
                                (*partIt).setElement( tmpPartElement );
                            }
                        }
                    }
                    (*messageIt).setParts( parts );
                }
            }
        }
    }

    definitions.setMessages( tmpMessages );
    Type tmpType = definitions.type();
    XSD::Types tmpTypes = tmpType.types();
    tmpTypes.setSimpleTypes( tmpSimpleTypes );
    tmpTypes.setComplexTypes( tmpComplexTypes );
    tmpTypes.setElements( tmpElements );
    tmpType.setTypes( tmpTypes );
    definitions.setType( tmpType );

////////////////////////////////// TODO END: disambiguate //////////////////////////////////

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

bool Converter::convert()
{
    convertTypes();
    //  mNSManager.dump();
    if (Settings::self()->generateServerCode()) {
        convertServerService();
    }
    return convertClientService();
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
}
