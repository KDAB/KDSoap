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

QVariant KDSoapPendingCall::Private::parseReplyElement(QXmlStreamReader& reader)
{
    //qDebug() << "parsing" << reader.name();
    KDSoapValueList lst;
    QString text;
    while (reader.readNext()) {
        if (reader.isEndElement())
            break;
        if (reader.isCharacters()) {
            text = reader.text().toString();
            //qDebug() << "text=" << text;
        } else if (reader.isStartElement()) {
            const QVariant subVal = parseReplyElement(reader);
            const QString name = reader.name().toString();
            lst.append(KDSoapValue(name, subVal));
        }
    }
    if (!lst.isEmpty())
        return QVariant::fromValue(lst);
    return text;
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
        if (doDebug)
            qDebug() << reply->errorString();
    } else {
        const QByteArray data = reply->readAll();
        if (doDebug)
            qDebug() << data;
        QXmlStreamReader reader(data);
        const QString soapNS = QString::fromLatin1("http://schemas.xmlsoap.org/soap/envelope/");
        //const QString xmlSchemaNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema");
        //const QString xmlSchemaInstanceNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema-instance");
        if (readNextStartElement(reader)) {
            if (reader.name() == "Envelope" && reader.namespaceUri() == soapNS) {
                if (readNextStartElement(reader) && reader.name() == "Body" && reader.namespaceUri() == soapNS) {

                    if (readNextStartElement(reader)) { // the method: Response or Fault
                        //qDebug() << "toplevel element:" << reader.name();
                        if (reader.name() == "Fault")
                            replyMessage.setFault(true);

                        while (readNextStartElement(reader)) { // Result
                            const QString name = reader.name().toString();
                            const QVariant val = parseReplyElement(reader);
                            replyMessage.addArgument(name, val);
                            if (doDebug)
                                qDebug() << "got item" << name << "val=" << val;
                        }
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
