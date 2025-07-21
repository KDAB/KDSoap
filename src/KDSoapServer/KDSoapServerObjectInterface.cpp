/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapServerObjectInterface.h"
#include "KDSoapClient/KDSoapValue.h"
#include "KDSoapServerSocket_p.h"
#include <QBuffer>
#include <QDebug>
#include <QFileInfo>
#include <QMetaEnum>
#include <QPointer>
#include <QResource>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "ByteArrayViewSplitter.h"
#endif

class KDSoapServerObjectInterface::Private
{
public:
    Private()
        : m_serverSocket(nullptr)
    {
    }

    KDSoapHeaders m_requestHeaders;
    KDSoapHeaders m_responseHeaders;
    QString m_faultCode;
    QString m_faultString;
    QString m_faultActor;
    QString m_detail;
    KDSoapValue m_detailValue;
    QString m_responseNamespace;
    QByteArray m_soapAction;
    KDSoap::SoapVersion m_requestVersion = KDSoap::SoapVersion::SOAP1_1;
    // QPointer in case the client disconnects during a delayed response
    QPointer<KDSoapServerSocket> m_serverSocket;
    QMap<QByteArray, QByteArray> m_httpHeaders;
};

KDSoapServerObjectInterface::HttpResponseHeaderItem::HttpResponseHeaderItem(const QByteArray &name, const QByteArray &value)
    : m_value(value)
    , m_name(name)
{
}

KDSoapServerObjectInterface::HttpResponseHeaderItem::HttpResponseHeaderItem()
{
}

KDSoapServerObjectInterface::KDSoapServerObjectInterface()
    : d(new Private)
{
}

KDSoapServerObjectInterface::~KDSoapServerObjectInterface()
{
    delete d;
}

void KDSoapServerObjectInterface::processRequest(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray &soapAction)
{
    const QString method = request.name();
    qDebug() << "Slot not found:" << method << "[soapAction =" << soapAction << "]" /* << "in" << metaObject()->className()*/;
    response.createFaultMessage(QString::fromLatin1("Server.MethodNotFound"), QString::fromLatin1("%1 not found").arg(method), d->m_requestVersion);
}

QIODevice *KDSoapServerObjectInterface::processFileRequest(const QString &path, QByteArray &contentType)
{
    Q_UNUSED(path);
    Q_UNUSED(contentType);
    return nullptr;
}

void KDSoapServerObjectInterface::processRequestWithPath(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray &soapAction,
                                                         const QString &path)
{
    Q_UNUSED(soapAction);
    const QString method = request.name();
    qWarning("Invalid path: \"%s\"", qPrintable(path));
    // qWarning() << "Invalid path:" << path << "[method =" << method << "; soapAction =" << soapAction << "]" /* << "in" <<
    // metaObject()->className();
    response.createFaultMessage(QString::fromLatin1("Client.Data"), QString::fromLatin1("Method %1 not found in path %2").arg(method, path),
                                d->m_requestVersion);
}

KDSoapServerObjectInterface::HttpResponseHeaderItems KDSoapServerObjectInterface::additionalHttpResponseHeaderItems() const
{
    return HttpResponseHeaderItems();
}

void KDSoapServerObjectInterface::doneProcessingRequestWithPath(const KDSoapServerObjectInterface &otherInterface)
{
    d->m_faultCode = otherInterface.d->m_faultCode;
    d->m_faultString = otherInterface.d->m_faultString;
    d->m_faultActor = otherInterface.d->m_faultActor;
    d->m_detail = otherInterface.d->m_detail;
    d->m_detailValue = otherInterface.d->m_detailValue;
    d->m_responseHeaders = otherInterface.d->m_responseHeaders;
    d->m_responseNamespace = otherInterface.d->m_responseNamespace;
}

void KDSoapServerObjectInterface::setFault(const QString &faultCode, const QString &faultString, const QString &faultActor, const QString &detail)
{
    Q_ASSERT(!faultCode.isEmpty());
    d->m_faultCode = faultCode;
    d->m_faultString = faultString;
    d->m_faultActor = faultActor;
    d->m_detail = detail;
}

void KDSoapServerObjectInterface::setFault(const QString &faultCode, const QString &faultString, const QString &faultActor, const KDSoapValue &detail)
{
    Q_ASSERT(!faultCode.isEmpty());
    d->m_faultCode = faultCode;
    d->m_faultString = faultString;
    d->m_faultActor = faultActor;
    d->m_detailValue = detail;
}

void KDSoapServerObjectInterface::storeFaultAttributes(KDSoapMessage &message) const
{
    // SOAP 1.1  <faultcode>, <faultstring>, <faultfactor>, <detail>
    message.addArgument(QString::fromLatin1("faultcode"), d->m_faultCode);
    message.addArgument(QString::fromLatin1("faultstring"), d->m_faultString);
    message.addArgument(QString::fromLatin1("faultactor"), d->m_faultActor);
    if (d->m_detailValue.isNil() || d->m_detailValue.isNull()) {
        message.addArgument(QString::fromLatin1("detail"), d->m_detail);
    } else {
        KDSoapValueList detailAsList;
        detailAsList.append(d->m_detailValue);
        message.addArgument(QString::fromLatin1("detail"), detailAsList);
    }
    // TODO  : Answer SOAP 1.2  <Code> , <Reason> , <Node> , <Role> , <Detail>
}

bool KDSoapServerObjectInterface::hasFault() const
{
    return !d->m_faultCode.isEmpty();
}

QAbstractSocket *KDSoapServerObjectInterface::serverSocket() const
{
    return d->m_serverSocket;
}

KDSoapHeaders KDSoapServerObjectInterface::requestHeaders() const
{
    return d->m_requestHeaders;
}

void KDSoapServerObjectInterface::setRequestHeaders(const KDSoapHeaders &headers, const QByteArray &soapAction)
{
    d->m_requestHeaders = headers;
    d->m_soapAction = soapAction;
    // Prepare for a new request to be handled
    d->m_faultCode.clear();
    d->m_responseHeaders.clear();
}

void KDSoapServerObjectInterface::setRequestVersion(KDSoap::SoapVersion requestVersion)
{
    d->m_requestVersion = requestVersion;
}

void KDSoapServerObjectInterface::setResponseHeaders(const KDSoapHeaders &headers)
{
    d->m_responseHeaders = headers;
}

KDSoapHeaders KDSoapServerObjectInterface::responseHeaders() const
{
    return d->m_responseHeaders;
}

QByteArray KDSoapServerObjectInterface::soapAction() const
{
    return d->m_soapAction;
}

KDSoapDelayedResponseHandle KDSoapServerObjectInterface::prepareDelayedResponse()
{
    return KDSoapDelayedResponseHandle(d->m_serverSocket);
}

void KDSoapServerObjectInterface::setServerSocket(KDSoapServerSocket *serverSocket)
{
    d->m_serverSocket = serverSocket;
    d->m_httpHeaders = d->m_serverSocket->m_httpHeaders;
}

void KDSoapServerObjectInterface::sendDelayedResponse(const KDSoapDelayedResponseHandle &responseHandle, const KDSoapMessage &response)
{
    KDSoapServerSocket *socket = responseHandle.serverSocket();
    if (socket) {
        socket->sendDelayedReply(this, response);
    }
}

namespace {
// Parses an Accept-Encoding header into a bitmask of EncodingFormat values.
//
// See RFC 9110 §12.5.3: Accept-Encoding
// Recognizes q-values (q=0 means "not acceptable") and wildcard "*" entries.
//
// Accept-Encoding: gzip, deflate, br, identity;q=0
// Accept-Encoding: *, identity;q=0  → means "anything except identity"
// Accept-Encoding: *;q=0, gzip      → means "anything except gzip"
KDSoapServerSocket::EncodingFormats parseAcceptEncoding(const QByteArray &headerValue)
{
    KDSoapServerSocket::EncodingFormats formats; // Accepted formats
    KDSoapServerSocket::EncodingFormats rejectedFormats; // Explicitly rejected formats (q=0)

    const QMetaEnum metaEnum = QMetaEnum::fromType<KDSoapServerSocket::EncodingFormats>();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    using PartType = QByteArrayView;
    const auto parts = ByteArrayViewSplitter(headerValue, ',', Qt::SkipEmptyParts);
#else
    using PartType = QByteArray;
    const auto parts = headerValue.split(',');
#endif

    for (const auto &rawPart : parts) {
        const auto part = rawPart.trimmed();
        if (part.isEmpty())
            continue;

        // Per RFC 9110 §12.5.3, we parse each `token[;q=value]`
        const int semiPos = part.indexOf(';');
        const auto encoding = semiPos >= 0 ? part.left(semiPos).trimmed() : part;
        const auto params = semiPos >= 0 ? part.mid(semiPos + 1).trimmed() : PartType();

        // Parse q= parameter (if present) looking for q=0 (rejection)
        bool rejected = false;
        if (params.startsWith("q=")) {
            bool ok = false;
            const float qVal = params.mid(2).trimmed().toFloat(&ok);
            if (ok && qVal == 0.0f) {
                rejected = true;
            }
        }

        // Handle wildcard "*" — add all known formats not explicitly rejected
        if (encoding == "*") {
            if (!rejected) {
                // Accept all known formats (per our enum) except already rejected ones
                for (int i = 0; i < metaEnum.keyCount(); ++i) {
                    const auto val = static_cast<KDSoapServerSocket::EncodingFormat>(metaEnum.value(i));
                    if (!rejectedFormats.testFlag(val))
                        formats |= val;
                }
            } else if (!formats.testFlag(KDSoapServerSocket::EncodingFormat::identity)) {
                // Reject all formats not specifically mentioned, including identity
                rejectedFormats |= KDSoapServerSocket::EncodingFormat::identity;
                qInfo() << __LINE__ << formats << rejectedFormats;
            }
            continue;
        }

        // Normalize encoding to lowercase and look up enum value
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const int value = metaEnum.keyToValue(QByteArray(encoding.constData(), encoding.size()).toLower().constData());
#else
        const int value = metaEnum.keyToValue(encoding.toLower().constData());
#endif
        if (value >= 0) {
            const auto format = static_cast<KDSoapServerSocket::EncodingFormats>(value);

            if (rejected) {
                // Remove from accepted in case it was added earlier (e.g. by *) and add to rejected
                formats &= ~format;
                rejectedFormats |= format;
            } else {
                // Accept format and remove from rejected
                formats |= format;
                rejectedFormats &= ~format;
            }
        }
        // Unknown encodings are ignored per RFC 9110 §12.5.3
    }

    // Ensure identity is accepted unless explicitly rejected
    if (!rejectedFormats.testFlag(KDSoapServerSocket::EncodingFormat::identity)) {
        formats |= KDSoapServerSocket::EncodingFormat::identity;
    }

    return formats;
}

// Convert QResource::Compression enum to our HTTP encoding enum
KDSoapServerSocket::EncodingFormat compressionToEncodingFormat(QResource::Compression c)
{
    switch (c) {
    case QResource::NoCompression:
        return KDSoapServerSocket::EncodingFormat::identity;
    case QResource::ZlibCompression:
        // Qt uses zlib internally, HTTP calls this "deflate" (RFC 7231 §4.2.2)
        return KDSoapServerSocket::EncodingFormat::deflate;
    case QResource::ZstdCompression:
        // zstd
        return KDSoapServerSocket::EncodingFormat::zstd;
    }
    return KDSoapServerSocket::EncodingFormat::identity;
}
}

std::pair<QIODevice *, QByteArray> KDSoapServerObjectInterface::fileForEncoding(const QString &logicalPath)
{
    QFileInfo fileInfo(logicalPath);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << logicalPath;
        return {nullptr, QByteArrayLiteral("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n")};
    }

    const QString resolvedPath = fileInfo.absoluteFilePath();
    const auto acceptedEncodings = parseAcceptEncoding(d->m_httpHeaders.value("accept-encoding"));

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    // Attempt to interpret as a Qt resource
    if (QResource res(resolvedPath); res.isValid()) {
        KDSoapServerSocket::EncodingFormat resourceCompression = compressionToEncodingFormat(res.compressionAlgorithm());

        // If resource is compressed and the format is accepted
        if (resourceCompression != KDSoapServerSocket::EncodingFormat::identity && acceptedEncodings.testFlag(resourceCompression)) {
            QBuffer *buffer = new QBuffer;
            // Per https://doc.qt.io/archives/qt-5.13/qresource.html and
            // https://doc.qt.io/archives/qt-5.13/qbytearray.html#qUncompress
            // the resource has a 4 byte header which contains the big endian uncompressed size.
            buffer->setData(reinterpret_cast<const char *>(res.data() + 4), res.size() - 4);

            // Return compressed content with Content-Encoding
            const auto metaEnum = QMetaEnum::fromType<KDSoapServerSocket::EncodingFormats>();
            return {buffer, QByteArray::fromRawData(metaEnum.valueToKey(resourceCompression), ::qstrlen(metaEnum.valueToKey(resourceCompression)))};
        }
    }
#endif

    // At this point:
    // - Either it's not a Qt resource, or
    // - It's a resource but compression was not accepted

    // RFC 9110 §8.4.4: "If the identity encoding is not acceptable, the origin server SHOULD send a 406 (Not Acceptable) response."
    if (!acceptedEncodings.testFlag(KDSoapServerSocket::EncodingFormat::identity)) {
        qWarning() << "Uncompressed file rejected by Accept-Encoding:" << resolvedPath;
        return {nullptr, QByteArrayLiteral("HTTP/1.1 406 Not Acceptable\r\nContent-Length: 0\r\n\r\n")};
    }

    // Fall back to serving uncompressed version via QFile
    QFile *file = new QFile(resolvedPath);

    return {file, QByteArray()};
}

void KDSoapServerObjectInterface::writeHTTP(const QByteArray &httpReply)
{
    const qint64 written = d->m_serverSocket->write(httpReply);
    Q_ASSERT(written == httpReply.size()); // Please report a bug if you hit this.
    Q_UNUSED(written);
}

void KDSoapServerObjectInterface::writeXML(const QByteArray &reply, bool isFault)
{
    d->m_serverSocket->writeXML(reply, isFault, d->m_requestVersion);
}

void KDSoapServerObjectInterface::copyFrom(KDSoapServerObjectInterface *other)
{
    d->m_requestHeaders = other->d->m_requestHeaders;
    d->m_soapAction = other->d->m_soapAction;
    d->m_serverSocket = other->d->m_serverSocket;
    d->m_httpHeaders = other->d->m_httpHeaders;
    d->m_requestVersion = other->d->m_requestVersion;
}

KDSoap::SoapVersion KDSoapServerObjectInterface::requestVersion() const
{
    return d->m_requestVersion;
}

void KDSoapServerObjectInterface::setResponseNamespace(const QString &ns)
{
    d->m_responseNamespace = ns;
}

QString KDSoapServerObjectInterface::responseNamespace() const
{
    return d->m_responseNamespace;
}
