#include "KDSoapMessage.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDDateTime.h"
#include <QDebug>
#include <QXmlStreamReader>
#include <QVariant>

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapMessageData()
        : use(KDSoapMessage::LiteralUse), isFault(false)
    {}

    KDSoapMessage::Use use;
    bool isFault;
};

KDSoapMessage::KDSoapMessage()
    : d(new KDSoapMessageData)
{
}

KDSoapMessage::KDSoapMessage(const KDSoapMessage& other)
    : KDSoapValue(other), d(other.d)
{
}

KDSoapMessage &KDSoapMessage::operator =(const KDSoapMessage &other)
{
    KDSoapValue::operator=(other);
    d = other.d;
    return *this;
}

KDSoapMessage &KDSoapMessage::operator =(const KDSoapValue &other)
{
    KDSoapValue::operator=(other);
    return *this;
}


KDSoapMessage::~KDSoapMessage()
{
}

void KDSoapMessage::addArgument(const QString &argumentName, const QVariant& argumentValue, const QString& typeNameSpace, const QString& typeName)
{
    childValues().append(KDSoapValue(argumentName, argumentValue, typeNameSpace, typeName));
}

void KDSoapMessage::addArgument(const QString& argumentName, const KDSoapValueList& argumentValueList, const QString& typeNameSpace, const QString& typeName)
{
    KDSoapValue soapValue(argumentName, argumentValueList, typeNameSpace, typeName);
    childValues().append(soapValue);
}

// I'm leaving the arguments() method even though it's the same as childValues,
// because it's the documented public API, needed even in the most simple case,
// while childValues is the "somewhat internal" KDSoapValue stuff.

KDSoapValueList& KDSoapMessage::arguments()
{
    return childValues();
}

const KDSoapValueList& KDSoapMessage::arguments() const
{
    return childValues();
}

QDebug operator <<(QDebug dbg, const KDSoapMessage &msg)
{
    return dbg << KDSoapValue(msg);
}

bool KDSoapMessage::isFault() const
{
    return d->isFault;
}

QString KDSoapMessage::faultAsString() const
{
    // This better be on a single line, since it's used by server-side logging too
    return QObject::tr("Fault code %1: %2 (%3)")
            .arg(childValues().child(QLatin1String("faultcode")).value().toString())
            .arg(childValues().child(QLatin1String("faultstring")).value().toString())
            .arg(childValues().child(QLatin1String("faultactor")).value().toString());
}

void KDSoapMessage::setFault(bool fault)
{
    d->isFault = fault;
}

KDSoapMessage::Use KDSoapMessage::use() const
{
    return d->use;
}

void KDSoapMessage::setUse(Use use)
{
    d->use = use;
}

////

static int xmlTypeToMetaType(const QString& xmlType)
{
    // Reverse operation from variantToXmlType in KDSoapClientInterface, keep in sync
    static const struct {
        const char* xml; // xsd: prefix assumed
        const int metaTypeId;
    } s_types[] = {
        { "string", QVariant::String }, // or QUrl
        { "base64Binary", QVariant::ByteArray },
        { "int", QVariant::Int }, // or long, or uint, or longlong
        { "unsignedInt", QVariant::ULongLong },
        { "boolean", QVariant::Bool },
        { "float", QMetaType::Float },
        { "double", QVariant::Double },
        { "time", QVariant::Time },
        { "date", QVariant::Date }
    };
    // Speed: could be sorted and then we could use qBinaryFind
    static const int s_numTypes = sizeof(s_types) / sizeof(*s_types);
    for (int i = 0; i < s_numTypes; ++i) {
        if (xmlType == QLatin1String(s_types[i].xml)) {
            return s_types[i].metaTypeId;
        }
    }
    if (xmlType == QLatin1String("dateTime"))
        return qMetaTypeId<KDDateTime>();
    // This will happen with any custom type, don't bother the user
    //qDebug() << QString::fromLatin1("xmlTypeToMetaType: XML type %1 is not supported in "
    //                                "KDSoap, see the documentation").arg(xmlType);
    return -1;
}

static QStringRef namespaceForPrefix(const QXmlStreamNamespaceDeclarations& decls, const QString& prefix)
{
    for (int i = 0; i < decls.count(); ++i) {
        const QXmlStreamNamespaceDeclaration& decl = decls.at(i);
        if (decl.prefix() == prefix)
            return decl.namespaceUri();
    }
    return QStringRef();
}

static KDSoapValue parseElement(QXmlStreamReader& reader, const QXmlStreamNamespaceDeclarations& envNsDecls)
{
    const QString name = reader.name().toString();
    KDSoapValue val(name, QVariant());
    val.setNamespaceUri(reader.namespaceUri().toString());
    //qDebug() << "parsing" << name;
    QVariant::Type metaTypeId = QVariant::Invalid;

    const QXmlStreamAttributes attributes = reader.attributes();
    Q_FOREACH(const QXmlStreamAttribute& attribute, attributes) {
        const QStringRef name = attribute.name();
        const QStringRef ns = attribute.namespaceUri();
        const QStringRef attrValue = attribute.value();
        // Parse xsi:type and soap-enc:arrayType
        // and ignore anything else from the xsi or soap-enc namespaces until someone needs it...
        if (ns == KDSoapNamespaceManager::xmlSchemaInstance1999() ||
            ns == KDSoapNamespaceManager::xmlSchemaInstance2001()) {
            if (name == QLatin1String("type")) {
                // The type can be like xsd:float, resolve that
                const QString type = attrValue.toString();
                const int pos = type.indexOf(QLatin1Char(':'));
                const QString dataType = type.mid(pos+1);
                val.setType(namespaceForPrefix(envNsDecls, type.left(pos)).toString(), dataType);
                metaTypeId = static_cast<QVariant::Type>(xmlTypeToMetaType(dataType));
            }
            continue;
        } else if (ns == KDSoapNamespaceManager::soapEncoding() || ns == KDSoapNamespaceManager::soapEnvelope()) {
            continue;
        }
        //qDebug() << "Got attribute:" << name << ns << "=" << attrValue;
        val.childValues().attributes().append(KDSoapValue(name.toString(), attrValue.toString()));
    }
    QString text;
    while (reader.readNext() != QXmlStreamReader::Invalid) {
        if (reader.isEndElement())
            break;
        if (reader.isCharacters()) {
            text = reader.text().toString();
            //qDebug() << "text=" << text;
        } else if (reader.isStartElement()) {
            const KDSoapValue subVal = parseElement(reader, envNsDecls); // recurse
            val.childValues().append(subVal);
        }
    }

    if (!text.isEmpty()) {
        QVariant variant(text);
        //qDebug() << text << variant << metaTypeId;
        // With use=encoded, we have type info, we can convert the variant here
        // Otherwise, for servers, we do it later, once we know the method's parameter types.
        if (metaTypeId != QVariant::Invalid) {
            QVariant copy = variant;
            if (!variant.convert(metaTypeId))
                variant = copy;
        }
        val.setValue(variant);
    }
    return val;
}

// Wrapper for compatibility with Qt < 4.6.
static bool readNextStartElement(QXmlStreamReader& reader)
{
#if QT_VERSION >= 0x040600
    return reader.readNextStartElement();
#else
    while (reader.readNext() != QXmlStreamReader::Invalid) {
        if (reader.isEndElement())
            return false;
        else if (reader.isStartElement())
            return true;
    }
    return false;
#endif
}

void KDSoapMessage::parseSoapXml(const QByteArray& data, QString* pMessageNamespace, KDSoapHeaders* pRequestHeaders)
{
    QXmlStreamReader reader(data);
    if (readNextStartElement(reader)) {
        if (reader.name() == "Envelope" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {
            const QXmlStreamNamespaceDeclarations envNsDecls = reader.namespaceDeclarations();
            if (readNextStartElement(reader)) {
                if (reader.name() == "Header" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {
                    while (readNextStartElement(reader)) {
                        KDSoapMessage header;
                        static_cast<KDSoapValue &>(header) = parseElement(reader, envNsDecls);
                        pRequestHeaders->append(header);
                    }
                    readNextStartElement(reader); // read <Body>
                }
                if (reader.name() == "Body" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {

                    if (reader.readNext() != QXmlStreamReader::Invalid) {
                        *this = parseElement(reader, envNsDecls);
                        if (pMessageNamespace)
                            *pMessageNamespace = namespaceUri();
                        if (name() == QLatin1String("Fault"))
                            setFault(true);
                    }

                } else {
                    reader.raiseError(QObject::tr("Invalid SOAP Message, Body expected"));
                }
            } else {
                reader.raiseError(QObject::tr("Invalid SOAP Message, empty Envelope"));
            }
        } else {
            reader.raiseError(QObject::tr("Invalid SOAP Message, Envelope expected"));
        }
    }
    if (reader.hasError()) {
        setFault(true);
        addArgument(QString::fromLatin1("faultcode"), QString::number(reader.error()));
        addArgument(QString::fromLatin1("faultstring"), QString::fromLatin1("XML error line %1: %2").arg(reader.lineNumber()).arg(reader.errorString()));
    }
}

KDSoapMessage KDSoapHeaders::header(const QString &name) const
{
    const_iterator it = begin();
    const const_iterator e = end();
    for ( ; it != e ; ++it) {
        if ((*it).name() == name)
            return *it;
    }
    return KDSoapMessage();
}
