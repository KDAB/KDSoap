#include "KDSoapPendingCall.h"
#include "KDSoapPendingCall_p.h"
#include "KDSoapMessage_p.h"
#include <QNetworkReply>
#include <QDebug>

KDSoapPendingCall::Private::~Private()
{
    delete reply.data();
    delete buffer;
}


KDSoapPendingCall::KDSoapPendingCall(QNetworkReply* reply, QBuffer* buffer)
    : d(new Private(reply, buffer))
{
}

KDSoapPendingCall::KDSoapPendingCall(const KDSoapPendingCall &other)
    : d(other.d)
{
}

KDSoapPendingCall::~KDSoapPendingCall()
{
}

KDSoapPendingCall &KDSoapPendingCall::operator=(const KDSoapPendingCall &other)
{
    d = other.d;
    return *this;
}

bool KDSoapPendingCall::isFinished() const
{
#if QT_VERSION >= 0x040600
    return d->reply.data()->isFinished();
#else
    return false;
#endif
}

KDSoapMessage KDSoapPendingCall::returnMessage() const
{
    d->parseReply();
    return d->replyMessage;
}

QVariant KDSoapPendingCall::returnValue() const
{
    d->parseReply();
    if (!d->replyMessage.d->args.isEmpty())
        return d->replyMessage.d->args.first().value();
    return QVariant();
}

// TODO a central repo of these known namespaces
static const char* xmlSchemaInstanceNS = "http://www.w3.org/1999/XMLSchema-instance";
static const char* xmlSchemaInstance2001NS = "http://www.w3.org/2001/XMLSchema-instance";
static const char* soapEncodingNS = "http://schemas.xmlsoap.org/soap/encoding/";
static const char* soapNS = "http://schemas.xmlsoap.org/soap/envelope/";

KDSoapValue KDSoapPendingCall::Private::parseReplyElement(QXmlStreamReader& reader)
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
        if (ns == QLatin1String(xmlSchemaInstanceNS) ||
            ns == QLatin1String(xmlSchemaInstance2001NS)) {
            if (name == QLatin1String("type")) {
                // TODO use reader.namespaceDeclarations() in the main loop, to be able to resolve namespaces
                val.setType(reader.namespaceUri().toString() /*wrong*/, attrValue.toString());
            }
            continue;
        } else if (ns == QLatin1String(soapEncodingNS) || ns == QLatin1String(soapNS)) {
            continue;
        }
        //qDebug() << "Got attribute:" << name << ns << "=" << attrValue;
        val.childValues().attributes().append(KDSoapValue(name.toString(), attrValue.toString()));
    }
    QString text;
    while (reader.readNext()) {
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

void KDSoapPendingCall::Private::parseReply()
{
    if (parsed)
        return;
    parsed = true;
    const bool doDebug = qgetenv("KDSOAP_DEBUG").toInt();
    QNetworkReply* reply = this->reply.data();
#if QT_VERSION >= 0x040600
    if (!reply->isFinished()) {
        qWarning("KDSoap: Parsing reply before it finished!");
    }
#endif
    if (reply->error()) {
        replyMessage.setFault(true);
        replyMessage.addArgument(QString::fromLatin1("faultcode"), QString::number(reply->error()));
        replyMessage.addArgument(QString::fromLatin1("faultstring"), reply->errorString());
        if (doDebug) {
            //qDebug() << reply->readAll();
            qDebug() << reply->errorString();
        }
    } else {
        const QByteArray data = reply->readAll();
        if (doDebug)
            qDebug() << data;
        QXmlStreamReader reader(data);
        const QString soapNS = QString::fromLatin1("http://schemas.xmlsoap.org/soap/envelope/");
        //const QString xmlSchemaNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema");
        if (readNextStartElement(reader)) {
            if (reader.name() == "Envelope" && reader.namespaceUri() == soapNS) {
                if (readNextStartElement(reader) && reader.name() == "Body" && reader.namespaceUri() == soapNS) {

                    if (readNextStartElement(reader)) { // the method: Response or Fault
                        //qDebug() << "toplevel element:" << reader.name();
                        if (reader.name() == "Fault")
                            replyMessage.setFault(true);

                        KDSoapValue val = parseReplyElement(reader);
                        replyMessage.arguments() = val.childValues();
                    }

                } else {
                    reader.raiseError(QObject::tr("Invalid SOAP Response, Body expected"));
                }
            } else {
                reader.raiseError(QObject::tr("Invalid SOAP Response, Envelope expected"));
            }
        }
        if (reader.hasError()) {
            replyMessage.setFault(true);
            replyMessage.addArgument(QString::fromLatin1("faultcode"), QString::number(reader.error()));
            replyMessage.addArgument(QString::fromLatin1("faultstring"), QString::fromLatin1("XML error line %1: %2").arg(reader.lineNumber()).arg(reader.errorString()));
        }
    }
}
