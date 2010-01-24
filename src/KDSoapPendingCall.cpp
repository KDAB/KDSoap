#include "KDSoapPendingCall.h"
#include "KDSoapPendingCall_p.h"
#include "KDSoapMessage_p.h"
#include <QNetworkReply>
#include <QDebug>
#include <QXmlStreamReader>

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

KDSoapMessage KDSoapPendingCall::returnMessage() const
{
    return d->replyMessage;
}

QVariant KDSoapPendingCall::returnValue() const
{
    if (!d->replyMessage.d->args.isEmpty())
        return d->replyMessage.d->args.first().value();
    return QVariant();
}

void KDSoapPendingCall::parseReply()
{
    const bool doDebug = qgetenv("KDSOAP_DEBUG").toInt();
    if (d->reply->error()) {
        d->replyMessage.setFault(true);
        d->replyMessage.addArgument(QString::fromLatin1("faultcode"), QString::number(d->reply->error()));
        d->replyMessage.addArgument(QString::fromLatin1("faultstring"), d->reply->errorString());
        if (doDebug)
            qDebug() << d->reply->errorString();
    } else {
        const QByteArray data = d->reply->readAll();
        if (doDebug)
            qDebug() << data;
        QXmlStreamReader reader(data);
        const QString soapNS = QString::fromLatin1("http://schemas.xmlsoap.org/soap/envelope/");
        //const QString xmlSchemaNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema");
        //const QString xmlSchemaInstanceNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema-instance");
        if (reader.readNextStartElement() && reader.name() == "Envelope" && reader.namespaceUri() == soapNS) {
            if (reader.readNextStartElement() && reader.name() == "Body" && reader.namespaceUri() == soapNS) {

                if (reader.readNextStartElement()) { // the method: Response or Fault
                    if (reader.name() == "Fault")
                        d->replyMessage.setFault(true);

                    while (reader.readNextStartElement()) { // Result
                        if (doDebug)
                            qDebug() << "got item" << reader.name().toString();
                        d->replyMessage.addArgument(reader.name().toString(), reader.readElementText());
                        //reader.skipCurrentElement();
                    }
                }

            } else {
                reader.raiseError(QObject::tr("Invalid SOAP Response, Body expected"));
            }
        } else {
            reader.raiseError(QObject::tr("Invalid SOAP Response, Envelope expected"));
        }
    }
}
