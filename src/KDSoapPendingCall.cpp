#include "KDSoapPendingCall.h"
#include "KDSoapPendingCall_p.h"
#include "KDSoapNamespaceManager.h"
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
    if (!d->replyMessage.childValues().isEmpty())
        return d->replyMessage.childValues().first().value();
    return QVariant();
}

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
        if (readNextStartElement(reader)) {
            if (reader.name() == "Envelope" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {
                if (readNextStartElement(reader) && reader.name() == "Body" && reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()) {

                    if (readNextStartElement(reader)) { // the method: Response or Fault
                        //qDebug() << "toplevel element:" << reader.name();
                        const bool isFault = (reader.name() == "Fault");

                        //replyMessage.KDSoapValue::operator=(parseReplyElement(reader));
                        static_cast<KDSoapValue &>(replyMessage) = parseReplyElement(reader);
                        if (isFault)
                            replyMessage.setFault(true);
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
