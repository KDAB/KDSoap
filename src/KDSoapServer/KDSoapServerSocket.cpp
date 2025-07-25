/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapServer.h"
#include "KDSoapServerAuthInterface.h"
#include "KDSoapServerCustomVerbRequestInterface.h"
#include "KDSoapServerObjectInterface.h"
#include "KDSoapServerRawXMLInterface.h"
#include "KDSoapServerSocket_p.h"
#include "KDSoapSocketList_p.h"
#include <KDSoapClient/KDSoapMessage.h>
#include <KDSoapClient/KDSoapMessageReader_p.h>
#include <KDSoapClient/KDSoapMessageWriter_p.h>
#include <KDSoapClient/KDSoapNamespaceManager.h>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaMethod>
#include <QThread>
#include <QVarLengthArray>

static const char s_forbidden[] = "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n";

KDSoapServerSocket::KDSoapServerSocket(KDSoapSocketList *owner, QObject *serverObject)
#ifndef QT_NO_SSL
    : QSslSocket()
    ,
#else
    : QTcpSocket()
    ,
#endif
    m_owner(owner)
    , m_serverObject(serverObject)
    , m_delayedResponse(false)
    , m_socketEnabled(true)
    , m_receivedData(false)
    , m_useRawXML(false)
    , m_bytesReceived(0)
    , m_chunkStart(0)
{
    connect(this, &QIODevice::readyRead, this, &KDSoapServerSocket::slotReadyRead);
    m_doDebug = qEnvironmentVariableIsSet("KDSOAP_DEBUG");
}

// The socket is deleted when it emits disconnected() (see KDSoapSocketList::handleIncomingConnection).
KDSoapServerSocket::~KDSoapServerSocket()
{
    // same as m_owner->socketDeleted, but safe in case m_owner is deleted first
    emit socketDeleted(this);

    delete m_serverObject;
}

typedef QMap<QByteArray, QByteArray> HeadersMap;
static HeadersMap parseHeaders(const QByteArray &headerData)
{
    HeadersMap headersMap;
    QBuffer sourceBuffer;
    sourceBuffer.setData(headerData);
    sourceBuffer.open(QIODevice::ReadOnly);
    // The first line is special, it's the GET or POST line
    const QList<QByteArray> firstLine = sourceBuffer.readLine().split(' ');
    if (firstLine.count() < 3) {
        qDebug() << "Malformed HTTP request:" << firstLine;
        return headersMap;
    }
    const QByteArray &requestType = firstLine.at(0);
    headersMap.insert("_requestType", requestType);

    // Grammar from https://datatracker.ietf.org/doc/html/rfc7230#section-5.3.1
    //  origin-form    = absolute-path [ "?" query ]
    // and https://datatracker.ietf.org/doc/html/rfc3986#section-3.3
    // says the path ends at the first '?' or '#' character
    const QByteArray arg1 = firstLine.at(1);
    const int queryPos = arg1.indexOf('?');
    const QByteArray path = queryPos >= 0 ? arg1.left(queryPos) : arg1;
    const QByteArray query = queryPos >= 0 ? arg1.mid(queryPos) : QByteArray();
    // Unfortunately QDir::cleanPath works with QString
    QByteArray cleanedPath = QDir::cleanPath(QString::fromUtf8(path)).toUtf8();
    // And unfortunately it keeps "//<host>/" unchanged on Windows
    while (cleanedPath.startsWith("//")) // while() so we also clean up "///"
        cleanedPath = cleanedPath.mid(1);
    headersMap.insert("_path", cleanedPath);
    headersMap.insert("_query", query);

    const QByteArray &httpVersion = firstLine.at(2);
    headersMap.insert("_httpVersion", httpVersion);

    while (!sourceBuffer.atEnd()) {
        const QByteArray line = sourceBuffer.readLine();
        const int pos = line.indexOf(':');
        if (pos == -1) {
            qDebug() << "Malformed HTTP header:" << line;
        }
        const QByteArray header = line.left(pos).toLower(); // RFC2616 section 4.2 "Field names are case-insensitive"
        const QByteArray value = line.mid(pos + 1).trimmed(); // remove space before and \r\n after
        // qDebug() << "HEADER" << header << "VALUE" << value;
        headersMap.insert(header, value);
    }
    return headersMap;
}

// We could parse headers as we go along looking for \r\n, and stop at empty header line, to avoid all this memory copying
// But in practice XML parsing (and writing) is far, far slower anyway.
static bool splitHeadersAndData(const QByteArray &request, QByteArray &header, QByteArray &data)
{
    const int sep = request.indexOf("\r\n\r\n");
    if (sep <= 0) {
        return false;
    }
    header = request.left(sep);
    data = request.mid(sep + 4);
    return true;
}

static QByteArray stripQuotes(const QByteArray &bar)
{
    if (bar.startsWith('\"') && bar.endsWith('\"')) {
        return bar.mid(1, bar.length() - 2);
    }

    return bar;
}

static QByteArray httpResponseHeaders(bool fault, const QByteArray &contentType, int responseDataSize, QObject *serverObject)
{
    QByteArray httpResponse;
    httpResponse.reserve(50);
    if (fault) {
        // https://www.w3.org/TR/2007/REC-soap12-part0-20070427 and look for 500
        httpResponse += "HTTP/1.1 500 Internal Server Error\r\n";
    } else if (responseDataSize == 0) {
        httpResponse += "HTTP/1.1 204 No Content\r\n";
    } else {
        httpResponse += "HTTP/1.1 200 OK\r\n";
    }

    httpResponse += "Content-Type: ";
    httpResponse += contentType;
    httpResponse += "\r\nContent-Length: ";
    httpResponse += QByteArray::number(responseDataSize);
    httpResponse += "\r\n";

    KDSoapServerObjectInterface *serverObjectInterface = qobject_cast<KDSoapServerObjectInterface *>(serverObject);
    if (serverObjectInterface) {
        const KDSoapServerObjectInterface::HttpResponseHeaderItems &additionalItems = serverObjectInterface->additionalHttpResponseHeaderItems();
        for (const KDSoapServerObjectInterface::HttpResponseHeaderItem &headerItem : std::as_const(additionalItems)) {
            httpResponse += headerItem.m_name;
            httpResponse += ": ";
            httpResponse += headerItem.m_value;
            httpResponse += "\r\n";
        }
    }

    httpResponse += "\r\n"; // end of headers
    return httpResponse;
}

void KDSoapServerSocket::slotReadyRead()
{
    if (!m_socketEnabled) {
        return;
    }

    // QNAM in Qt 5.x tends to connect additional sockets in advance and not use them
    // So only count the sockets which actually sent us data (for the servertest unittest).
    if (!m_receivedData) {
        m_receivedData = true;
        m_owner->increaseConnectionCount();
    }

    // qDebug() << this << QThread::currentThread() << "slotReadyRead!";

    QByteArray buf(2048, ' ');
    qint64 nread = -1;
    while (nread != 0) {
        nread = read(buf.data(), buf.size());
        if (nread < 0) {
            qDebug() << "Error reading from server socket:" << errorString();
            return;
        }
        m_requestBuffer += buf.left(nread);
        m_bytesReceived += nread;
    }

    KDSoapServerRawXMLInterface *rawXmlInterface = qobject_cast<KDSoapServerRawXMLInterface *>(m_serverObject);

    if (m_httpHeaders.isEmpty()) {
        // New request: see if we can parse headers
        QByteArray receivedHttpHeaders, receivedData;
        const bool splitOK = splitHeadersAndData(m_requestBuffer, receivedHttpHeaders, receivedData);
        if (!splitOK) {
            // qDebug() << "Incomplete SOAP request, wait for more data";
            // incomplete request, wait for more data
            return;
        }
        m_httpHeaders = parseHeaders(receivedHttpHeaders);
        // Leave only the actual data in the buffer
        m_requestBuffer = receivedData;
        m_bytesReceived = receivedData.size();
        m_useRawXML = false;
        if (rawXmlInterface) {
            KDSoapServerObjectInterface *serverObjectInterface = qobject_cast<KDSoapServerObjectInterface *>(m_serverObject);
            serverObjectInterface->setServerSocket(this);
            m_useRawXML = rawXmlInterface->newRequest(m_httpHeaders.value("_requestType"), m_httpHeaders);
        }
    }

    if (m_doDebug) {
        qDebug() << "headers:" << m_httpHeaders;
        qDebug() << "data received:" << m_requestBuffer;
    }

    if (m_httpHeaders.value("transfer-encoding") != "chunked") {
        if (m_useRawXML) {
            rawXmlInterface->processXML(m_requestBuffer);
            m_requestBuffer.clear();
        }

        const QByteArray contentLength = m_httpHeaders.value("content-length");
        if (m_bytesReceived < contentLength.toInt()) {
            return; // incomplete request, wait for more data
        }

        if (m_useRawXML) {
            rawXmlInterface->endRequest();
        } else {
            handleRequest(m_httpHeaders, m_requestBuffer);
        }
    } else {
        // qDebug() << "requestBuffer has " << m_requestBuffer.size() << "bytes, starting at" << m_chunkStart;
        while (m_chunkStart >= 0) {
            const int nextEOL = m_requestBuffer.indexOf("\r\n", m_chunkStart);
            if (nextEOL == -1) {
                return;
            }
            const QByteArray chunkSizeStr = m_requestBuffer.mid(m_chunkStart, nextEOL - m_chunkStart);
            // qDebug() << m_chunkStart << nextEOL << "chunkSizeStr=" << chunkSizeStr;
            bool ok;
            int chunkSize = chunkSizeStr.toInt(&ok, 16);
            if (!ok) {
                const QByteArray badRequest = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
                write(badRequest);
                return;
            }
            if (chunkSize == 0) { // done!
                m_requestBuffer = m_requestBuffer.mid(nextEOL);
                m_chunkStart = -1;
                break;
            }
            if (nextEOL + 2 + chunkSize + 2 >= m_requestBuffer.size()) {
                return; // not enough data, chunk is incomplete
            }
            const QByteArray chunk = m_requestBuffer.mid(nextEOL + 2, chunkSize);
            if (m_useRawXML) {
                rawXmlInterface->processXML(chunk);
            } else {
                m_decodedRequestBuffer += chunk;
            }
            m_chunkStart = nextEOL + 2 + chunkSize + 2;
        }
        // We have the full data, now ensure we read trailers
        if (!m_requestBuffer.contains("\r\n\r\n")) {
            return;
        }
        if (m_useRawXML) {
            rawXmlInterface->endRequest();
        } else {
            handleRequest(m_httpHeaders, m_decodedRequestBuffer);
        }
        m_decodedRequestBuffer.clear();
        m_chunkStart = 0;
    }
    m_requestBuffer.clear();
    m_httpHeaders.clear();
    m_receivedData = false;
}

// We're working in a virtual filesystem here, we have no physical root dir nor a concept of symlinks
// So all we can check is that the path doesn't contain so many "../" that we're going out of the virtual root
static bool isPathSecure(const QString &path)
{
    // The input path has already gone through cleanPath, so we just need to check it doesn't start with ..
    if (!path.startsWith(QLatin1Char('/')))
        return false;
    if (path.startsWith(QLatin1String("/..")))
        return false;
    return true;
}

void KDSoapServerSocket::handleRequest(const QMap<QByteArray, QByteArray> &httpHeaders, const QByteArray &receivedData)
{
    const QString path = QString::fromUtf8(httpHeaders.value("_path").constData());
    if (!isPathSecure(path)) {
        // denied for security reasons
        write(s_forbidden);
        return;
    }

    const QByteArray requestType = httpHeaders.value("_requestType");
    const QString query = QString::fromUtf8(httpHeaders.value("_query").constData());
    const QString pathAndQuery = path + query;

    KDSoapServerAuthInterface *serverAuthInterface = qobject_cast<KDSoapServerAuthInterface *>(m_serverObject);
    if (serverAuthInterface) {
        const QByteArray authValue = httpHeaders.value("authorization");
        if (!serverAuthInterface->handleHttpAuth(authValue, pathAndQuery)) {
            // send auth request (Qt supports basic, ntlm and digest)
            const QByteArray unauthorized =
                "HTTP/1.1 401 Authorization Required\r\nWWW-Authenticate: Basic realm=\"example\"\r\nContent-Length: 0\r\n\r\n";
            write(unauthorized);
            return;
        }
    }

    if (requestType != "GET" && requestType != "POST") {
        KDSoapServerCustomVerbRequestInterface *serverCustomRequest = qobject_cast<KDSoapServerCustomVerbRequestInterface *>(m_serverObject);
        QByteArray customVerbRequestAnswer;
        if (serverCustomRequest && serverCustomRequest->processCustomVerbRequest(requestType, receivedData, httpHeaders, customVerbRequestAnswer)) {
            write(customVerbRequestAnswer);
            return;
        } else {
            qWarning() << "Unknown HTTP request:" << requestType;
            // handleError(replyMsg, "Client.Data", QString::fromLatin1("Invalid request type '%1', should be GET or
            // POST").arg(QString::fromLatin1(requestType.constData()))); sendReply(0, replyMsg);
            const QByteArray methodNotAllowed = "HTTP/1.1 405 Method Not Allowed\r\nAllow: GET POST\r\nContent-Length: 0\r\n\r\n";
            write(methodNotAllowed);
            return;
        }
    }

    KDSoapServer *server = m_owner->server();
    KDSoapMessage replyMsg;
    replyMsg.setUse(server->use());

    KDSoapServerObjectInterface *serverObjectInterface = qobject_cast<KDSoapServerObjectInterface *>(m_serverObject);
    if (!serverObjectInterface) {
        const QString error = QString::fromLatin1("Server object %1 does not implement KDSoapServerObjectInterface!")
                                  .arg(QString::fromLatin1(m_serverObject->metaObject()->className()));
        handleError(replyMsg, "Server.ImplementationError", error);
        sendReply(nullptr, replyMsg);
        return;
    } else {
        serverObjectInterface->setServerSocket(this);
    }

    if (requestType == "GET") {
        if (pathAndQuery == server->wsdlPathInUrl() && handleWsdlDownload()) {
            return;
        } else if (handleFileDownload(serverObjectInterface, pathAndQuery)) {
            return;
        }

        // See https://www.ibm.com/developerworks/xml/library/x-tipgetr/
        // We could implement it, but there's no SOAP request, just a query in the URL,
        // which we'd have to pass to a different virtual than processRequest.
        handleError(replyMsg, "Client.Data", QString::fromLatin1("Support for GET requests not implemented yet."));
        sendReply(nullptr, replyMsg);
        return;
    }

    // parse message
    KDSoapMessage requestMsg;
    KDSoapHeaders requestHeaders;
    KDSoapMessageReader reader;
    KDSoapMessageReader::XmlError err = reader.xmlToMessage(receivedData, &requestMsg, &m_messageNamespace, &requestHeaders, KDSoap::SOAP1_1);
    if (err == KDSoapMessageReader::PrematureEndOfDocumentError) {
        // qDebug() << "Incomplete SOAP message, wait for more data";
        // This should never happen, since we check for content-size above.
        return;
    } // TODO handle parse errors?

    // check soap version and extract soapAction header
    KDSoap::SoapVersion soapVersion = KDSoap::SoapVersion::SOAP1_1;
    QByteArray soapAction;
    const QByteArray contentType = httpHeaders.value("content-type");
    if (contentType.startsWith("text/xml")) { // krazy:exclude=strings
        // SOAP 1.1
        soapVersion = KDSoap::SoapVersion::SOAP1_1;
        soapAction = httpHeaders.value("soapaction");
        // The SOAP standard allows quotation marks around the SoapAction, so we have to get rid of these.
        soapAction = stripQuotes(soapAction);

    } else if (contentType.startsWith("application/soap+xml")) { // krazy:exclude=strings
        // SOAP 1.2
        soapVersion = KDSoap::SoapVersion::SOAP1_2;
        // Example: application/soap+xml;charset=utf-8;action=ActionHex
        const QList<QByteArray> parts = contentType.split(';');
        for (const QByteArray &part : std::as_const(parts)) {
            if (part.trimmed().startsWith("action=")) { // krazy:exclude=strings
                soapAction = stripQuotes(part.mid(part.indexOf('=') + 1));
            }
        }
    }

    m_method = requestMsg.name();

    if (!replyMsg.isFault()) {
        makeCall(serverObjectInterface, requestMsg, replyMsg, requestHeaders, soapAction, pathAndQuery, soapVersion);
    }

    if (serverObjectInterface && m_delayedResponse) {
        // Delayed response. Disable the socket to make sure we don't handle another call at the same time.
        setSocketEnabled(false);
    } else {
        sendReply(serverObjectInterface, replyMsg);
    }
}

bool KDSoapServerSocket::handleWsdlDownload()
{
    KDSoapServer *server = m_owner->server();
    const QString wsdlFile = server->wsdlFile();
    QFile wf(wsdlFile);
    if (wf.open(QIODevice::ReadOnly)) {
        // qDebug() << "Returning wsdl file contents";
        const QByteArray responseText = wf.readAll();
        const QByteArray response = httpResponseHeaders(false, "application/xml", responseText.size(), m_serverObject);
        write(response);
        write(responseText);
        return true;
    }
    return false;
}

bool KDSoapServerSocket::handleFileDownload(KDSoapServerObjectInterface *serverObjectInterface, const QString &path)
{
    QByteArray contentType;
    QIODevice *device = serverObjectInterface->processFileRequest(path, contentType);
    if (!device) {
        const QByteArray notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        write(notFound);
        return true;
    }
    if (!device->open(QIODevice::ReadOnly)) {
        write(s_forbidden);
        delete device;
        return true; // handled!
    }
    const QByteArray response = httpResponseHeaders(false, contentType, device->size(), m_serverObject);
    if (m_doDebug) {
        qDebug() << "KDSoapServerSocket: file download response" << response;
    }
    qint64 written = write(response);
    Q_ASSERT(written == response.size()); // Please report a bug if you hit this.
    Q_UNUSED(written);

    char block[4096] = {0};
    // qint64 totalRead = 0;
    while (!device->atEnd()) {
        const qint64 in = device->read(block, sizeof(block));
        if (in <= 0) {
            break;
        }
        // totalRead += in;
        if (in != write(block, in)) {
            // error = true;
            break;
        }
    }
    // if (totalRead != device->size()) {
    //    // Unable to read from the source.
    //    error = true;
    //}

    delete device;
    // TODO log the file request, if logging is enabled?
    return true;
}

void KDSoapServerSocket::writeXML(const QByteArray &xmlResponse, bool isFault, KDSoap::SoapVersion soapVersion)
{
    const QByteArray httpHeaders = httpResponseHeaders(isFault, soapVersion == KDSoap::SoapVersion::SOAP1_1 ? "text/xml" : "application/soap+xml;charset=utf-8",
                                                       xmlResponse.size(), m_serverObject);
    if (m_doDebug) {
        qDebug() << "KDSoapServerSocket: writing" << httpHeaders << xmlResponse;
    }
    qint64 written = write(httpHeaders);
    if (written != httpHeaders.size()) {
        qWarning() << "Only wrote" << written << "out of" << httpHeaders.size() << "bytes of HTTP headers. Error:" << errorString();
    }
    written = write(xmlResponse);
    if (written != xmlResponse.size()) {
        qWarning() << "Only wrote" << written << "out of" << xmlResponse.size() << "bytes of response. Error:" << errorString();
    }
}

void KDSoapServerSocket::sendReply(KDSoapServerObjectInterface *serverObjectInterface, const KDSoapMessage &replyMsg)
{
    const bool isFault = replyMsg.isFault();

    QByteArray xmlResponse;
    if (!replyMsg.isNil()) {
        KDSoapMessageWriter msgWriter;
        msgWriter.setVersion(serverObjectInterface->requestVersion());
        // Note that the kdsoap client parsing code doesn't care for the name (except if it's fault), even in
        // Document mode. Other implementations do, though.
        QString responseName = isFault ? QString::fromLatin1("Fault") : replyMsg.name();
        if (responseName.isEmpty()) {
            responseName = m_method;
        }
        QString responseNamespace = m_messageNamespace;
        KDSoapHeaders responseHeaders;
        if (serverObjectInterface) {
            responseHeaders = serverObjectInterface->responseHeaders();
            if (!serverObjectInterface->responseNamespace().isEmpty()) {
                responseNamespace = serverObjectInterface->responseNamespace();
            }
        }
        msgWriter.setMessageNamespace(responseNamespace);
        xmlResponse = msgWriter.messageToXml(replyMsg, responseName, responseHeaders, QMap<QString, KDSoapMessage>());
    }

    writeXML(xmlResponse, isFault, serverObjectInterface->requestVersion());

    // All done, check if we should log this
    KDSoapServer *server = m_owner->server();
    const KDSoapServer::LogLevel logLevel =
        server->logLevel(); // we do this here in order to support dynamic settings changes (at the price of a mutex)
    if (logLevel != KDSoapServer::LogNothing) {
        if (logLevel == KDSoapServer::LogEveryCall || (logLevel == KDSoapServer::LogFaults && isFault)) {

            if (isFault) {
                server->log("FAULT " + m_method.toLatin1() + " -- " + replyMsg.faultAsString().toUtf8() + '\n');
            } else {
                server->log("CALL " + m_method.toLatin1() + '\n');
            }
        }
    }
}

void KDSoapServerSocket::sendDelayedReply(KDSoapServerObjectInterface *serverObjectInterface, const KDSoapMessage &replyMsg)
{
    sendReply(serverObjectInterface, replyMsg);
    m_delayedResponse = false;
    setSocketEnabled(true);
}

void KDSoapServerSocket::setResponseDelayed()
{
    m_delayedResponse = true;
}

void KDSoapServerSocket::handleError(KDSoapMessage &replyMsg, const char *errorCode, const QString &error, KDSoap::SoapVersion soapVersion)
{
    qWarning("%s", qPrintable(error));
    replyMsg.createFaultMessage(QString::fromLatin1(errorCode), error, soapVersion);
}

void KDSoapServerSocket::makeCall(KDSoapServerObjectInterface *serverObjectInterface, const KDSoapMessage &requestMsg, KDSoapMessage &replyMsg,
                                  const KDSoapHeaders &requestHeaders, const QByteArray &soapAction, const QString &path, KDSoap::SoapVersion soapVersion)
{
    Q_ASSERT(serverObjectInterface);

    if (requestMsg.isFault()) {
        // Can this happen? Getting a fault as a request !? Doesn't make sense...
        // reply with a fault, but we don't even know what main element name to use
        // Oh well, just use the incoming fault :-)
        replyMsg = requestMsg;
        handleError(replyMsg, "Client.Data", QString::fromLatin1("Request was a fault"), soapVersion);
    } else {

        // Call method on m_serverObject
        serverObjectInterface->setRequestHeaders(requestHeaders, soapAction);
        serverObjectInterface->setRequestVersion(soapVersion);

        KDSoapServer *server = m_owner->server();
        if (path != server->path()) {
            serverObjectInterface->processRequestWithPath(requestMsg, replyMsg, soapAction, path);
        } else {
            serverObjectInterface->processRequest(requestMsg, replyMsg, soapAction);
        }
        if (serverObjectInterface->hasFault()) {
            // qDebug() << "Got fault!";
            replyMsg.setFault(true);
            serverObjectInterface->storeFaultAttributes(replyMsg);
        }
    }
}

// Prevention against concurrent requests without waiting for a (delayed) reply,
// but untestable with QNAM on the client side, since it doesn't do that.
void KDSoapServerSocket::setSocketEnabled(bool enabled)
{
    if (m_socketEnabled == enabled) {
        return;
    }

    m_socketEnabled = enabled;
    if (enabled) {
        slotReadyRead();
    }
}

#include "moc_KDSoapServerSocket_p.cpp"
