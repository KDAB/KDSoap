#include "KDSoapMessage.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapNamespacePrefixes_p.h"
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
    return QObject::tr("Fault code: %1\nFault description: %2 (%3)")
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
        { "date", QVariant::Date },
        { "dateTime", QVariant::DateTime }
    };
    // Speed: could be sorted and then we could use qBinaryFind
    static const int s_numTypes = sizeof(s_types) / sizeof(*s_types);
    for (int i = 0; i < s_numTypes; ++i) {
        if (xmlType == QLatin1String(s_types[i].xml)) {
            return s_types[i].metaTypeId;
        }
    }
    qDebug() << QString::fromLatin1("xmlTypeToVariant: XML type %1 is not supported in "
                                    "KDSoap, see the documentation").arg(xmlType);
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

    QVariant variant(text);
    if (metaTypeId != QVariant::Invalid) {
        variant.convert(metaTypeId);
    }
    val.setValue(variant);
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

void KDSoapMessage::parseSoapXml(const QByteArray& data, QString* pMessageNamespace)
{
    QXmlStreamReader reader(data);
    if (readNextStartElement(reader)) {
        if (reader.name() == "Envelope" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {
            const QXmlStreamNamespaceDeclarations envNsDecls = reader.namespaceDeclarations();
            if (readNextStartElement(reader) && reader.name() == "Body" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {

                if (readNextStartElement(reader)) { // the root element: request, response or fault
                    //qDebug() << "toplevel element:" << reader.name();
                    const bool isFault = (reader.name() == "Fault");

                    //KDSoapValue::operator=(parseReplyElement(reader));
                    if (pMessageNamespace)
                        *pMessageNamespace = reader.namespaceUri().toString();
                    static_cast<KDSoapValue &>(*this) = parseElement(reader, envNsDecls);
                    if (isFault)
                        setFault(true);
                }

            } else {
                reader.raiseError(QObject::tr("Invalid SOAP Response, Body expected"));
            }
        } else {
            reader.raiseError(QObject::tr("Invalid SOAP Response, Envelope expected"));
        }
    }
    if (reader.hasError()) {
        setFault(true);
        addArgument(QString::fromLatin1("faultcode"), QString::number(reader.error()));
        addArgument(QString::fromLatin1("faultstring"), QString::fromLatin1("XML error line %1: %2").arg(reader.lineNumber()).arg(reader.errorString()));
    }
}

