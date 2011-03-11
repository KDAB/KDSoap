#include "KDSoapClientInterface.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapNamespaceManager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDebug>
#include <QBuffer>
#include <QXmlStreamWriter>
#include <QDateTime>

KDSoapClientInterface::KDSoapClientInterface(const QString& endPoint, const QString& messageNamespace)
    : d(new Private)
{
    d->m_endPoint = endPoint;
    d->m_messageNamespace = messageNamespace;
    d->m_version = SOAP1_1;
}

KDSoapClientInterface::~KDSoapClientInterface()
{
    d->m_thread.stop();
    d->m_thread.wait();
    delete d;
}

void KDSoapClientInterface::setSoapVersion(KDSoapClientInterface::SoapVersion version)
{
    d->m_version = version;
}

KDSoapClientInterface::SoapVersion KDSoapClientInterface::soapVersion()
{
  return d->m_version;
}

static QString variantToTextValue(const QVariant& value)
{
    switch (value.userType())
    {
    case QVariant::Char:
        // fall-through
    case QVariant::String:
        return value.toString();
    case QVariant::Url:
        // xmlpatterns/data/qatomicvalue.cpp says to do this:
        return value.toUrl().toString();
    case QVariant::ByteArray:
        {
            const QByteArray b64 = value.toByteArray().toBase64();
            return QString::fromLatin1(b64.data(),b64.size());
        }
    case QVariant::Int:
        // fall-through
    case QVariant::LongLong:
        // fall-through
    case QVariant::UInt:
        return QString::number(value.toLongLong());
    case QVariant::ULongLong:
        return QString::number(value.toULongLong());
    case QVariant::Bool:
    case QMetaType::Float:
    case QVariant::Double:
        return value.toString();
    case QVariant::Time:
    {
        const QTime time = value.toTime();
        if (time.msec()) {
            // include milli-seconds
            return time.toString(QLatin1String("hh:mm:ss.zzz"));
        } else {
            return time.toString(Qt::ISODate);
        }
    }
    case QVariant::Date:
        return value.toDate().toString(Qt::ISODate);
    case QVariant::DateTime:
    {
        const QDateTime dt = value.toDateTime();
        const QTime time = dt.time();
        if (time.msec()) {
            // include milli-seconds
            return dt.toString(QLatin1String("yyyy-MM-ddThh:mm:ss.zzz"));
        } else {
            return dt.toString(Qt::ISODate);
        }
    }
    case QVariant::Invalid:
        qDebug() << "ERROR: Got invalid QVariant in a KDSoapValue";
        return QString();
    default:
        if (value.userType() == qMetaTypeId<float>())
            return QString::number(value.value<float>());

        qDebug() << QString::fromLatin1("QVariants of type %1 are not supported in "
                                        "KDSoap, see the documentation").arg(QLatin1String(value.typeName()));
        return value.toString();
    }
}

static QString variantToXMLType(const QVariant& value)
{
    switch (value.userType())
    {
    case QVariant::Char:
        // fall-through
    case QVariant::String:
        // fall-through
    case QVariant::Url:
        return QLatin1String("xsd:string");
    case QVariant::ByteArray:
        return QLatin1String("xsd:base64Binary");
    case QVariant::Int:
        // fall-through
    case QVariant::LongLong:
        // fall-through
    case QVariant::UInt:
        return QLatin1String("xsd:int");
    case QVariant::ULongLong:
        return QLatin1String("xsd:unsignedInt");
    case QVariant::Bool:
        return QLatin1String("xsd:boolean");
    case QMetaType::Float:
        return QLatin1String("xsd:float");
    case QVariant::Double:
        return QLatin1String("xsd:double");
    case QVariant::Time:
        return QLatin1String("xsd:time"); // correct? xmlpatterns fallsback to datetime because of missing timezone
    case QVariant::Date:
        return QLatin1String("xsd:date");
    case QVariant::DateTime:
        return QLatin1String("xsd:dateTime");
    default:
        if (value.userType() == qMetaTypeId<float>())
            return QLatin1String("xsd:float");

        qDebug() << value;

        qDebug() << QString::fromLatin1("variantToXmlType: QVariants of type %1 are not supported in "
                                        "KDSoap, see the documentation").arg(QLatin1String(value.typeName()));
        return QString();
    }
}

KDSoapClientInterface::Private::Private()
    : m_authentication(), m_ignoreSslErrors(false)
{
    connect(&m_accessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(_kd_slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
}

QNetworkRequest KDSoapClientInterface::Private::prepareRequest(const QString &method, const QString& action)
{
    QNetworkRequest request(QUrl(this->m_endPoint));

    // The soap action seems to be namespace + method in most cases, but not always
    // (e.g. urn:GoogleSearchAction for google).
    QString soapAction = action;
    if (soapAction.isEmpty()) {
        // Does the namespace always end with a '/'?
        soapAction = this->m_messageNamespace + /*QChar::fromLatin1('/') +*/ method;
    }
    //qDebug() << "soapAction=" << soapAction;

    QString soapHeader;
    if (m_version == SOAP1_1) {
        soapHeader += QString::fromLatin1("text/xml;charset=utf-8");
        request.setRawHeader("SoapAction", soapAction.toUtf8());
    } else if (m_version == SOAP1_2) {
        soapHeader += QString::fromLatin1("application/soap+xml;charset=utf-8;action=") + soapAction;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, soapHeader.toUtf8());

    // FIXME need to find out which version of Qt this is no longer necessary
    // without that the server might respond with gzip compressed data and
    // Qt 4.6.2 fails to decode that properly
    //
    // happens with retrieval calls in against SugarCRM 5.5.1 running on Apache 2.2.15
    // when the response seems to reach a certain size threshold
    request.setRawHeader( "Accept-Encoding", "compress" );

    return request;
}

class KDSoapNamespacePrefixes : public QMap<QString /*ns*/, QString /*prefix*/>
{
public:
    void writeNamespace(QXmlStreamWriter& writer, const QString& ns, const QString& prefix) {
        //qDebug() << "writeNamespace" << ns << prefix;
        insert(ns, prefix);
        writer.writeNamespace(ns, prefix);
    }
    QString resolve(const QString& ns, const QString& localName) const {
        const QString prefix = value(ns);
        if (prefix.isEmpty()) {
            qWarning("ERROR: Namespace not found: %s (for localName %s)", qPrintable(ns), qPrintable(localName));
        }
        return prefix + QLatin1Char(':') + localName;
    }
};

QBuffer* KDSoapClientInterface::Private::prepareRequestBuffer(const QString& method, const KDSoapMessage& message, const KDSoapHeaders& headers)
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.writeStartDocument();

    KDSoapNamespacePrefixes namespacePrefixes;

    const QString soapNS = KDSoapNamespaceManager::soapEnvelope();
    namespacePrefixes.writeNamespace(writer, soapNS, QLatin1String("soap"));
    namespacePrefixes.writeNamespace(writer, KDSoapNamespaceManager::soapEncoding(), QLatin1String("soap-enc"));
    namespacePrefixes.writeNamespace(writer, KDSoapNamespaceManager::xmlSchema1999(), QLatin1String("xsd"));
    namespacePrefixes.writeNamespace(writer, KDSoapNamespaceManager::xmlSchemaInstance1999(), QLatin1String("xsi"));

    // Also insert known variants
    namespacePrefixes.insert(KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("xsd"));
    namespacePrefixes.insert(KDSoapNamespaceManager::xmlSchemaInstance2001(), QString::fromLatin1("xsi"));

    writer.writeStartElement(soapNS, QLatin1String("Envelope"));
    writer.writeAttribute(soapNS, QLatin1String("encodingStyle"), KDSoapNamespaceManager::soapEncoding());

    if (!headers.isEmpty() || !m_persistentHeaders.isEmpty()) {
        // This writeNamespace line adds the xmlns:n1 to <Envelope>, which looks ugly and unusual (and breaks all unittests)
        // However it's the best solution in case of headers, otherwise we get n1 in the header and n2 in the body,
        // and xsi:type attributes that refer to n1, which isn't defined in the body...
        namespacePrefixes.writeNamespace(writer, this->m_messageNamespace, QLatin1String("n1") /*make configurable?*/);
        writer.writeStartElement(soapNS, QLatin1String("Header"));
        Q_FOREACH(const KDSoapMessage& header, m_persistentHeaders) {
            writeChildren(namespacePrefixes, writer, header.childValues(), header.use());
        }
        Q_FOREACH(const KDSoapMessage& header, headers) {
            writeChildren(namespacePrefixes, writer, header.childValues(), header.use());
        }
        writer.writeEndElement(); // Header
    } else {
        // So in the standard case (no headers) we just rely on Qt calling it n1 and insert it into the map.
        // Calling this after the writeStartElement(method) below leads to a double-definition of n1.
        namespacePrefixes.insert(this->m_messageNamespace, QString::fromLatin1("n1"));
    }

    writer.writeStartElement(soapNS, QLatin1String("Body"));

    writer.writeStartElement(this->m_messageNamespace, method);
    writeElementContents(namespacePrefixes, writer, message, message.use());
    writer.writeEndElement(); // <method>

    writer.writeEndElement(); // Body
    writer.writeEndElement(); // Envelope
    writer.writeEndDocument();

    if (qgetenv("KDSOAP_DEBUG").toInt()) {
        qDebug() << data;
    }

    QBuffer* buffer = new QBuffer;
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);
    return buffer;
}

void KDSoapClientInterface::Private::writeElementContents(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValue& element, KDSoapMessage::Use use)
{
    const QVariant value = element.value();
    const KDSoapValueList list = element.childValues();
    if (use == KDSoapMessage::EncodedUse) {
        // use=encoded means writing out xsi:type attributes. http://www.eherenow.com/soapfight.htm taught me that.
        QString type;
        if (!element.type().isEmpty())
            type = namespacePrefixes.resolve(element.typeNs(), element.type());
        if (type.isEmpty() && !value.isNull())
            type = variantToXMLType(value); // fallback
        if (!type.isEmpty()) {
            writer.writeAttribute(KDSoapNamespaceManager::xmlSchemaInstance1999(), QLatin1String("type"), type);
        }

        const bool isArray = !list.arrayType().isEmpty();
        if (isArray) {
            writer.writeAttribute(KDSoapNamespaceManager::soapEncoding(), QLatin1String("arrayType"), namespacePrefixes.resolve(list.arrayTypeNs(), list.arrayType()) + QLatin1Char('[') + QString::number(list.count()) + QLatin1Char(']'));
        }
    }
    writeChildren(namespacePrefixes, writer, list, use);

    if (!value.isNull())
        writer.writeCharacters(variantToTextValue(value));
}

void KDSoapClientInterface::Private::writeChildren(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValueList& args, KDSoapMessage::Use use)
{
    writeAttributes(writer, args.attributes());
    KDSoapValueListIterator it(args);
    while (it.hasNext()) {
        const KDSoapValue& element = it.next();
        writer.writeStartElement(this->m_messageNamespace, element.name());
        writeElementContents(namespacePrefixes, writer, element, use);
        writer.writeEndElement();
    }
}

void KDSoapClientInterface::Private::writeAttributes(QXmlStreamWriter& writer, const QList<KDSoapValue>& attributes)
{
    Q_FOREACH(const KDSoapValue& attr, attributes) {
        Q_ASSERT(!attr.value().isNull());
        writer.writeAttribute(m_messageNamespace, attr.name(), variantToTextValue(attr.value()));
    }
}


KDSoapPendingCall KDSoapClientInterface::asyncCall(const QString &method, const KDSoapMessage &message, const QString& soapAction, const KDSoapHeaders& headers)
{
    QBuffer* buffer = d->prepareRequestBuffer(method, message, headers);
    QNetworkRequest request = d->prepareRequest(method, soapAction);
    QNetworkReply* reply = d->m_accessManager.post(request, buffer);
    d->setupReply(reply);
    return KDSoapPendingCall(reply, buffer);
}

KDSoapMessage KDSoapClientInterface::call(const QString& method, const KDSoapMessage &message, const QString& soapAction, const KDSoapHeaders& headers)
{
    // Problem is: I don't want a nested event loop here. Too dangerous for GUI programs.
    // I wanted a socket->waitFor... but we don't have access to the actual socket in QNetworkAccess.
    // So the only option that remains is a thread and acquiring a semaphore...
    KDSoapThreadTaskData* task = new KDSoapThreadTaskData(this, method, message, soapAction, headers);
    task->m_authentication = d->m_authentication;
    d->m_thread.enqueue(task);
    if (!d->m_thread.isRunning())
        d->m_thread.start();
    task->waitForCompletion();
    KDSoapMessage ret = task->returnArguments();
    delete task;
    return ret;
}

void KDSoapClientInterface::callNoReply(const QString &method, const KDSoapMessage &message, const QString &soapAction, const KDSoapHeaders& headers)
{
    QBuffer* buffer = d->prepareRequestBuffer(method, message, headers);
    QNetworkRequest request = d->prepareRequest(method, soapAction);
    QNetworkReply* reply = d->m_accessManager.post(request, buffer);
    d->setupReply(reply);
    QObject::connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void KDSoapClientInterface::Private::_kd_slotAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    m_authentication.handleAuthenticationRequired(reply, authenticator);
}

void KDSoapClientInterface::setAuthentication(const KDSoapAuthentication &authentication)
{
    d->m_authentication = authentication;
}

void KDSoapClientInterface::setHeader(const QString& name, const KDSoapMessage &header)
{
    d->m_persistentHeaders[name] = header;
}

void KDSoapClientInterface::ignoreSslErrors()
{
    d->m_ignoreSslErrors = true;
}

void KDSoapClientInterface::Private::setupReply(QNetworkReply *reply)
{
    if (m_ignoreSslErrors) {
        QObject::connect(reply, SIGNAL(sslErrors(const QList<QSslError>&)), reply, SLOT(ignoreSslErrors()));
    }
}

#include "moc_KDSoapClientInterface_p.cpp"
