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

KDSoapMessage KDSoapPendingCall::returnArguments() const
{
    return d->replyMessage;
}

QVariant KDSoapPendingCall::returnValue() const
{
    // Might be related to the hack below; but it's also what QtSoap did:
    // assume the return struct contains only one value
    // (I provided returnArguments() in case it doesn't)
    return d->replyMessage.d->args.first().value();
}

void KDSoapPendingCall::parseReply()
{
    const QByteArray data = d->reply->readAll();
    qDebug() << data;
    QXmlStreamReader reader(data);
    const QString soapNS = QString::fromLatin1("http://schemas.xmlsoap.org/soap/envelope/");
    const QString xmlSchemaNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema");
    const QString xmlSchemaInstanceNS = QString::fromLatin1("http://www.w3.org/1999/XMLSchema-instance");
    if (reader.readNextStartElement() && reader.name() == "Envelope" && reader.namespaceUri() == soapNS) {
        if (reader.readNextStartElement() && reader.name() == "Body" && reader.namespaceUri() == soapNS) {

            // TODO now read the rest into a "struct"
            // For now, this hack:
            if (reader.readNextStartElement()) { // Response or Fault
                if (reader.name() == "Fault")
                    d->replyMessage.setFault(true);

                while (reader.readNextStartElement()) { // Result
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
