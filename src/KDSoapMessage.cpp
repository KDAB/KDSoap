#include "KDSoapMessage.h"
#include "KDSoapNamespaceManager.h"
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

static KDSoapValue parseReplyElement(QXmlStreamReader& reader)
{
    const QString name = reader.name().toString();
    KDSoapValue val(name, QVariant());
    //qDebug() << "parsing" << name;

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
                // TODO use reader.namespaceDeclarations() in the main loop, to be able to resolve namespaces
                val.setType(reader.namespaceUri().toString() /*wrong*/, attrValue.toString());
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
            val.setValue(text);
        } else if (reader.isStartElement()) {
            const KDSoapValue subVal = parseReplyElement(reader);
            val.childValues().append(subVal);
        }
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

void KDSoapMessage::parseSoapXml(const QByteArray& data)
{
    QXmlStreamReader reader(data);
    if (readNextStartElement(reader)) {
        if (reader.name() == "Envelope" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {
            if (readNextStartElement(reader) && reader.name() == "Body" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {

                if (readNextStartElement(reader)) { // the method: Response or Fault
                    //qDebug() << "toplevel element:" << reader.name();
                    const bool isFault = (reader.name() == "Fault");

                    //KDSoapValue::operator=(parseReplyElement(reader));
                    static_cast<KDSoapValue &>(*this) = parseReplyElement(reader);
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
