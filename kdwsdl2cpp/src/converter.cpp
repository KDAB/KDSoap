#include "settings.h"
#include "elementargumentserializer.h"
#include "converter.h"
#include <libkode/style.h>
#include <QDebug>

using namespace KWSDL;

QString upperlize(const QString &str)
{
    // Upper the first letter, but also convert forbidden chars like '-' to '_'
    // So basically the same as:
    return KODE::Style::className(str);
}

QString lowerlize(const QString &str)
{
    const QString s = KODE::Style::className(str); // handle special chars
    return s.at(0).toLower() + s.mid(1);
}

QString namespaceString(const QString &ns)
{
    if (ns == QLatin1String("http://www.w3.org/1999/XMLSchema")) {
        return QLatin1String("KDSoapNamespaceManager::xmlSchema1999()");
    }
    if (ns == QLatin1String("http://www.w3.org/2001/XMLSchema")) {
        return QLatin1String("KDSoapNamespaceManager::xmlSchema2001()");
    }
    //qDebug() << "got namespace" << ns;
    // TODO register into KDSoapNamespaceManager? This means generating code in the clientinterface ctor...
    return QLatin1String("QString::fromLatin1(\"") + ns + QLatin1String("\")");
}

Converter::Converter()
    : mQObject(KODE::Class(QLatin1String("QObject"))),
      mKDSoapServerObjectInterface(KODE::Class(QLatin1String("KDSoapServerObjectInterface")))
{
}

void Converter::setWSDL(const WSDL &wsdl)
{
    mWSDL = wsdl;

    // Keep the prefixes from the wsdl parsing, they are more meaningful than ns1 :)
    mNSManager = wsdl.namespaceManager();

    // overwrite some default prefixes
    mNSManager.setPrefix(QLatin1String("soapenc"), QLatin1String("http://schemas.xmlsoap.org/soap/encoding/"));
    mNSManager.setPrefix(QLatin1String("http"), QLatin1String("http://schemas.xmlsoap.org/wsdl/http/"));
    mNSManager.setPrefix(QLatin1String("soap"), QLatin1String("http://schemas.xmlsoap.org/wsdl/soap/"));
    mNSManager.setPrefix(QLatin1String("xsd"), QLatin1String("http://www.w3.org/2001/XMLSchema"));
    mNSManager.setPrefix(QLatin1String("xsi"), QLatin1String("http://www.w3.org/2001/XMLSchema-instance"));

    // overwrite with prefixes from settings
    Settings::NSMapping mapping = Settings::self()->namespaceMapping();
    Settings::NSMapping::Iterator it;
    for (it = mapping.begin(); it != mapping.end(); ++it) {
        mNSManager.setPrefix(it.value(), it.key());
    }

    if (qgetenv("KDSOAP_TYPE_DEBUG").toInt()) {
        mNSManager.dump();
    }

    mTypeMap.setNSManager(&mNSManager);

    cleanupUnusedTypes();

    // set the xsd types
    mTypeMap.addSchemaTypes(mWSDL.definitions().type().types(), Settings::self()->nameSpace());

    if (qgetenv("KDSOAP_TYPE_DEBUG").toInt()) {
        mTypeMap.dump();
    }
}

class TypeCollector
{
public:
    TypeCollector(XSD::Types &allTypes, const QSet<QName> &usedTypes)
        : m_allTypes(allTypes),
          m_allUsedTypes(usedTypes)
    {}

    // In case of inheritance, the parser simply set the base class in the derived class.
    // We need to register the other way round (list of derived classes in a base class)
    // in order to make up a tree that can be navigated down, so that we collect all derived
    // classes that the application might want to use where a base class is expected
    // (rather than clean them up as unused)
    void registerDerivedClasses()
    {
        XSD::ComplexType::List complexTypes = m_allTypes.complexTypes();
        Q_FOREACH (const XSD::ComplexType &derivedType, complexTypes) {
            const QName base = derivedType.baseTypeName();
            if (!base.isEmpty()) {
                // Look for the base class and register type. Linear search, maybe we should use a QHash...
                for (int i = 0; i < complexTypes.count(); ++i) {
                    XSD::ComplexType &t = complexTypes[i];
                    if (base == t.qualifiedName())
                        //qDebug() << "Adding derived type" << derivedType.name() << "to base" << base;
                    {
                        t.addDerivedType(derivedType.qualifiedName());
                    }
                }
            }
        }
        m_allTypes.setComplexTypes(complexTypes);
    }

    void collectDependentTypes(const TypeMap &typeMap)
    {
        QSet<QName> typesToProcess = m_allUsedTypes;
        do {
            m_alsoUsedTypes.clear();
            Q_FOREACH (const QName &typeName, typesToProcess) {
                if (typeName.isEmpty()) {
                    continue;
                }
                if (typeMap.isBuiltinType(typeName)) {
                    continue;
                }
                //qDebug() << "used type:" << typeName;
                XSD::ComplexType complexType = m_allTypes.complexType(typeName);
                if (!complexType.name().isEmpty()) { // found it as a complex type
                    fixupComplexType(complexType);
                    usedComplexTypes.insert(typeName, complexType);

                    addDependency(complexType.baseTypeName());
                    Q_FOREACH (const QName &derivedTypeName, complexType.derivedTypes()) {
                        addDependency(derivedTypeName);
                    }

                    Q_FOREACH (const XSD::Element &element, complexType.elements()) {
                        addDependency(element.type());
                    }
                    Q_FOREACH (const XSD::Attribute &attribute, complexType.attributes()) {
                        addDependency(attribute.type());
                    }
                    addDependency(complexType.arrayType());

                } else {
                    XSD::SimpleType simpleType = m_allTypes.simpleType(typeName);
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

    QHash<QName, XSD::ComplexType> usedComplexTypes;
    XSD::SimpleType::List usedSimpleTypes;
    XSD::Types &m_allTypes;

private:
    void addDependency(const QName &type)
    {
        if (!type.isEmpty() && !m_allUsedTypes.contains(type) && !m_alsoUsedTypes.contains(type)) {
            m_alsoUsedTypes.insert(type);
            m_allUsedTypes.insert(type);
        }
    }

    void fixupComplexType(XSD::ComplexType &type)
    {
        // Check for conflicts (complex type Foo and anonymous complex type named after element foo, will both generate a class Foo later on)
        if (type.name().at(0).isLower()) {
            QName uppercaseType(type.nameSpace(), upperlize(type.name()));
            XSD::ComplexType upperType = usedComplexTypes.value(uppercaseType);
            if (!upperType.isNull()) {
                //qDebug() << "FIXUP: found" << uppercaseType << "already in usedComplexTypes";
                type.setConflicting(true);
            }
        } else {
            QName lowercaseType(type.nameSpace(), lowerlize(type.name()));
            XSD::ComplexType lowerType = usedComplexTypes.value(lowercaseType);
            if (!lowerType.isNull()) {
                //qDebug() << "FIXUP: found" << lowercaseType << "already in usedComplexTypes";
                usedComplexTypes.remove(lowercaseType);
                lowerType.setConflicting(true);
                usedComplexTypes.insert(lowercaseType, lowerType);
            }
        }
    }

    QSet<QName> m_allUsedTypes; // All already seen types
    QSet<QName> m_alsoUsedTypes; // The list of types to process in the next iteration
};

class MessageCollector
{
public:
    MessageCollector() {}

    QSet<QName> collectMessages(const WSDL &wsdl)
    {
        Q_FOREACH (const Service &service, wsdl.definitions().services()) {
            Q_FOREACH (const Port &port, service.ports()) {
                Binding binding = wsdl.findBinding(port.bindingName());
                //portTypeNames.insert( binding.portTypeName() );
                //qDebug() << "binding" << port.bindingName() << binding.name() << "port type" << binding.portTypeName();
                PortType portType = wsdl.findPortType(binding.portTypeName());
                const Operation::List operations = portType.operations();
                //qDebug() << "portType" << portType.name() << operations.count() << "operations";
                Q_FOREACH (const Operation &operation, operations) {
                    //qDebug() << "  operation" << operation.operationType() << operation.name();
                    switch (operation.operationType()) {
                    case Operation::OneWayOperation:
                        addMessage(operation.input().message());
                        break;
                    case Operation::RequestResponseOperation:
                    case Operation::SolicitResponseOperation:
                        addMessage(operation.input().message());
                        addMessage(operation.output().message());
                        break;
                    case Operation::NotificationOperation:
                        addMessage(operation.output().message());
                        break;
                    };
                    if (binding.type() == Binding::SOAPBinding) {
                        const SoapBinding soapBinding(binding.soapBinding());
                        const SoapBinding::Operation op = soapBinding.operations().value(operation.name());
                        Q_FOREACH (const SoapBinding::Header &header, op.inputHeaders()) {
                            addMessage(header.message());
                        }
                        Q_FOREACH (const SoapBinding::Header &header, op.outputHeaders()) {
                            addMessage(header.message());
                        }
                    }
                }
            }
        }
        return m_usedMessageNames;
    }
private:
    void addMessage(const QName &messageName)
    {
        Q_ASSERT(!messageName.isEmpty());
        m_usedMessageNames.insert(messageName);
    }

    QSet<QName> m_usedMessageNames;

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
        //Q_FOREACH(const XSD::ComplexType& complexType, types.complexTypes()) {
        //    qDebug() << "complex type:" << complexType.qualifiedName();
        //}
    }

    MessageCollector messageCollector;
    QSet<QName> usedMessageNames = messageCollector.collectMessages(mWSDL);
    //QSet<QName> portTypeNames;

    // Keep only the messages in usedMessageNames
    QSet<QName> usedTypes;
    QSet<QString> usedTypesStrings; // for debug
    QSet<QName> usedElementNames;
    Message::List newMessages;
    Q_FOREACH (const QName &messageName, usedMessageNames) {
        //qDebug() << "used message:" << messageName;
        Message message = mWSDL.findMessage(messageName);
        newMessages.append(message);
        Q_FOREACH (const Part &part, message.parts()) {
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
    TypeCollector collector(types, usedTypes);
    collector.registerDerivedClasses();
    collector.collectDependentTypes(mTypeMap);

    XSD::Element::List usedElements;
    QSetIterator<QName> elemIt(usedElementNames);
    while (elemIt.hasNext()) {
        const QName name = elemIt.next();
        XSD::Element element = mWSDL.findElement(name);

        if (element.type().isEmpty()) {
            qDebug() << "ERROR: Element without type:" << element.qualifiedName() << element.nameSpace() << element.name();
            Q_ASSERT(!element.type().isEmpty());
        }

        if (element.name().isEmpty()) {
            qDebug() << "cleanupUnusedTypes: element" << name << "not found";
        } else {
            usedElements.append(element);
        }
    }

    definitions.setMessages(newMessages);
    types.setComplexTypes(collector.usedComplexTypes.values());
    types.setSimpleTypes(collector.usedSimpleTypes);
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
        //Q_FOREACH(const XSD::ComplexType& complexType, types.complexTypes()) {
        //    qDebug() << "complex type:" << complexType.qualifiedName();
        //}
    }
}

KODE::Class::List Converter::classes() const
{
    return mClasses;
}

QString Converter::shortenFilename(const QString &path)
{
    return path.section(QLatin1Char('/'), -1);
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
    for (int i = 0; i < simpleTypes.count(); ++i) {
        convertSimpleType(&(simpleTypes[ i ]), simpleTypes);
    }

    XSD::ComplexType::List complexTypes = types.complexTypes();
    qDebug() << "Converting" << complexTypes.count() << "complex types";
    for (int i = 0; i < complexTypes.count(); ++i) {
        convertComplexType(&(complexTypes[ i ]));
    }
}

// Helper for clientstub and serverstub
KODE::Code Converter::serializePart(const Part &part, const QString &localVariableName, const QString &varName, bool append)
{
    bool qualified, nillable;
    const QName elemName = elementNameForPart(part, &qualified, &nillable);
    ElementArgumentSerializer serializer(mTypeMap, part.type(), part.element(), localVariableName);
    serializer.setElementName(elemName);
    serializer.setOutputVariable(varName, append);
    serializer.setIsQualified(qualified);
    serializer.setNillable(nillable);
    serializer.setOmitIfEmpty(false);   // Don't omit entire parts, this especially breaks the wrappers for RPC messages
    return serializer.generate();
}
