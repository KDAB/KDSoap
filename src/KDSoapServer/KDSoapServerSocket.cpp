/****************************************************************************
** Copyright (C) 2010-2014 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#include "KDSoapServerSocket_p.h"
#include "KDSoapSocketList_p.h"
#include "KDSoapServerObjectInterface.h"
#include "KDSoapServerAuthInterface.h"
#include "KDSoapServer.h"
#include <KDSoapClient/KDSoapMessage.h>
#include <KDSoapClient/KDSoapNamespaceManager.h>
#include <KDSoapClient/KDSoapMessageReader_p.h>
#include <KDSoapClient/KDSoapMessageWriter_p.h>
#include <QBuffer>
#include <QThread>
#include <QMetaMethod>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QVarLengthArray>

KDSoapServerSocket::KDSoapServerSocket(KDSoapSocketList* owner, QObject* serverObject)
#ifndef QT_NO_OPENSSL
    : QSslSocket(),
#else
    : QTcpSocket(),
#endif
      m_owner(owner),
      m_serverObject(serverObject),
      m_delayedResponse(false),
      m_socketEnabled(true)
{
    connect(this, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
    m_doDebug = qgetenv("KDSOAP_DEBUG").toInt();
}

// The socket is deleted when it emits disconnected() (see KDSoapSocketList::handleIncomingConnection).
KDSoapServerSocket::~KDSoapServerSocket()
{
    // same as m_owner->socketDeleted, but safe in case m_owner is deleted first
    emit socketDeleted(this);
}

typedef QMap<QByteArray, QByteArray> HeadersMap;
static HeadersMap parseHeaders(const QByteArray& headerData)
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
    const QByteArray requestType = firstLine.at(0);
    const QByteArray path = QDir::cleanPath(QString::fromLatin1(firstLine.at(1).constData())).toLatin1();
    const QByteArray httpVersion = firstLine.at(2);
    headersMap.insert("_requestType", requestType);
    headersMap.insert("_path", path);
    headersMap.insert("_httpVersion", httpVersion);

    while (!sourceBuffer.atEnd()) {
        const QByteArray line = sourceBuffer.readLine();
        const int pos = line.indexOf(':');
        if (pos == -1)
            qDebug() << "Malformed HTTP header:" << line;
        const QByteArray header = line.left(pos).toLower(); // RFC2616 section 4.2 "Field names are case-insensitive"
        const QByteArray value = line.mid(pos+1).trimmed(); // remove space before and \r\n after
        //qDebug() << "HEADER" << header << "VALUE" << value;
        headersMap.insert(header, value);
    }
    return headersMap;
}

// We could parse headers as we go along looking for \r\n, and stop at empty header line, to avoid all this memory copying
// But in practice XML parsing (and writing) is far far slower anyway.
static bool splitHeadersAndData(const QByteArray& request, QByteArray& header, QByteArray& data)
{
    const int sep = request.indexOf("\r\n\r\n");
    if (sep <= 0)
        return false;
    header = request.left(sep);
    data = request.mid(sep + 4);
    return true;
}

static QByteArray httpResponseHeaders(bool fault, const QByteArray& contentType, int responseDataSize)
{
    QByteArray httpResponse;
    httpResponse.reserve(50);
    if (fault) {
        // http://www.w3.org/TR/2007/REC-soap12-part0-20070427 and look for 500
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

    httpResponse += "\r\n"; // end of headers
    return httpResponse;
}

void KDSoapServerSocket::slotReadyRead()
{
    if (!m_socketEnabled)
        return;

    //qDebug() << this << QThread::currentThread() << "slotReadyRead!";
    QByteArray buf(2048, ' ');
    qint64 nread = -1;
    while (nread != 0) {
        nread = read(buf.data(), buf.size());
        if (nread < 0) {
            qDebug() << "Error reading from server socket:" << errorString();
            return;
        }
        m_requestBuffer += buf.left(nread);
    }

    //qDebug() << "KDSoapServerSocket: request:" << m_requestBuffer;

    QByteArray receivedHttpHeaders, receivedData;
    const bool splitOK = splitHeadersAndData(m_requestBuffer, receivedHttpHeaders, receivedData);

    if (!splitOK) {
        //qDebug() << "Incomplete SOAP request, wait for more data";
        //incomplete request, wait for more data
        return;
    }

    QMap<QByteArray, QByteArray> httpHeaders;
    httpHeaders = parseHeaders(receivedHttpHeaders);

    if (m_doDebug) {
        qDebug() << "headers received:" << receivedHttpHeaders;
        qDebug() << httpHeaders;
        qDebug() << "data received:" << receivedData;
    }

    const QByteArray contentLength = httpHeaders.value("content-length");
    if (receivedData.size() < contentLength.toInt())
        return; // incomplete request, wait for more data
    m_requestBuffer.clear();

    const QByteArray requestType = httpHeaders.value("_requestType");
    if (requestType != "GET" && requestType != "POST") {
        qWarning() << "Unknown HTTP request:" << requestType;
        //handleError(replyMsg, "Client.Data", QString::fromLatin1("Invalid request type '%1', should be GET or POST").arg(QString::fromLatin1(requestType.constData())));
        //sendReply(0, replyMsg);
        const QByteArray methodNotAllowed = "HTTP/1.1 405 Method Not Allowed\r\nAllow: GET POST\r\nContent-Length: 0\r\n\r\n";
        write(methodNotAllowed);
        return;
    }

    const QString path = QString::fromLatin1(httpHeaders.value("_path").constData());

    KDSoapServerAuthInterface* serverAuthInterface = qobject_cast<KDSoapServerAuthInterface *>(m_serverObject);
    if (serverAuthInterface) {
        QByteArray authValue = httpHeaders.value("Authorization");
        if (authValue.isEmpty())
            authValue = httpHeaders.value("authorization"); // as sent by Qt-4.5
        if (!serverAuthInterface->handleHttpAuth(authValue, path)) {
            // send auth request (Qt supports basic, ntlm and digest)
            const QByteArray unauthorized = "HTTP/1.1 401 Authorization Required\r\nWWW-Authenticate: Basic realm=\"example\"\r\nContent-Length: 0\r\n\r\n";
            write(unauthorized);
            return;
        }
    }

    KDSoapServer* server = m_owner->server();
    KDSoapMessage replyMsg;
    replyMsg.setUse(server->use());

    KDSoapServerObjectInterface* serverObjectInterface = qobject_cast<KDSoapServerObjectInterface *>(m_serverObject);
    if (!serverObjectInterface) {
        const QString error = QString::fromLatin1("Server object %1 does not implement KDSoapServerObjectInterface!").arg(QString::fromLatin1(m_serverObject->metaObject()->className()));
        handleError(replyMsg, "Server.ImplementationError", error);
        sendReply(0, replyMsg);
        return;
    } else {
        serverObjectInterface->setServerSocket(this);
    }

    if (requestType == "GET") {
        if (path == server->wsdlPathInUrl() && handleWsdlDownload()) {
            return;
        } else if (handleFileDownload(serverObjectInterface, path)) {
            return;
        }

        // See http://www.ibm.com/developerworks/xml/library/x-tipgetr/
        // We could implement it, but there's no SOAP request, just a query in the URL,
        // which we'd have to pass to a different virtual than processRequest.
        handleError(replyMsg, "Client.Data", QString::fromLatin1("Support for GET requests not implemented yet."));
        sendReply(0, replyMsg);
        return;
    }

    //parse message
    KDSoapMessage requestMsg;
    KDSoapHeaders requestHeaders;
    KDSoapMessageReader reader;
    KDSoapMessageReader::XmlError err = reader.xmlToMessage(receivedData, &requestMsg, &m_messageNamespace, &requestHeaders);
    if (err == KDSoapMessageReader::PrematureEndOfDocumentError) {
        //qDebug() << "Incomplete SOAP message, wait for more data";
        // This should never happen, since we check for content-size above.
        return;
    } //TODO handle parse errors?

    // check soap version and extract soapAction header
    QByteArray soapAction;
    const QByteArray contentType = httpHeaders.value("content-type");
    if (contentType.startsWith("text/xml")) { //krazy:exclude=strings
        // SOAP 1.1
        soapAction = httpHeaders.value("soapaction");
        // The SOAP standard allows quotation marks around the SoapAction, so we have to get rid of these.
        if (soapAction.startsWith('\"'))
            soapAction = soapAction.mid(1, soapAction.length() - 2);

    } else if (contentType.startsWith("application/soap+xml")) { //krazy:exclude=strings
        // SOAP 1.2
        // Example: application/soap+xml;charset=utf-8;action=ActionHex
        const QList<QByteArray> parts = contentType.split(';');
        Q_FOREACH(const QByteArray& part, parts) {
            if (part.startsWith("action=")) { //krazy:exclude=strings
                soapAction = part.mid(strlen("action="));
            }
        }
    }

    m_method = requestMsg.name();

    if (!replyMsg.isFault()) {
        makeCall(serverObjectInterface, requestMsg, replyMsg, requestHeaders, soapAction, path);
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
    KDSoapServer* server = m_owner->server();
    const QString wsdlFile = server->wsdlFile();
    QFile wf(wsdlFile);
    if (wf.open(QIODevice::ReadOnly)) {
        //qDebug() << "Returning wsdl file contents";
        const QByteArray responseText = wf.readAll();
        const QByteArray response = httpResponseHeaders(false, "application/xml", responseText.size());
        write(response);
        write(responseText);
        return true;
    }
    return false;
}

bool KDSoapServerSocket::handleFileDownload(KDSoapServerObjectInterface *serverObjectInterface, const QString &path)
{
    QByteArray contentType;
    QIODevice* device = serverObjectInterface->processFileRequest(path, contentType);
    if (!device) {
        const QByteArray notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        write(notFound);
        return true;
    }
    if (!device->open(QIODevice::ReadOnly)) {
        const QByteArray forbidden = "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n";
        write(forbidden);
        delete device;
        return true; // handled!
    }
    const QByteArray response = httpResponseHeaders(false, contentType, device->size());
    if (m_doDebug) {
        qDebug() << "KDSoapServerSocket: file download response" << response;
    }
    qint64 written = write(response);
    Q_ASSERT(written == response.size()); // Please report a bug if you hit this.
    Q_UNUSED(written);

    char block[4096];
    qint64 totalRead = 0;
    while (!device->atEnd()) {
        const qint64 in = device->read(block, sizeof(block));
        if (in <= 0)
            break;
        totalRead += in;
        if(in != write(block, in)) {
            //error = true;
            break;
        }
    }
    //if (totalRead != device->size()) {
    //    // Unable to read from the source.
    //    error = true;
    //}

    delete device;
    // TODO log the file request, if logging is enabled?
    return true;
}

void KDSoapServerSocket::sendReply(KDSoapServerObjectInterface* serverObjectInterface, const KDSoapMessage& replyMsg)
{
    const bool isFault = replyMsg.isFault();

    QByteArray xmlResponse;
    if (!replyMsg.isNull()) {
        KDSoapMessageWriter msgWriter;
        // Note that the kdsoap client parsing code doesn't care for the name (except if it's fault), even in
        // Document mode. Other implementations do, though.
        QString responseName = isFault ? QString::fromLatin1("Fault") : replyMsg.name();
        if (responseName.isEmpty())
            responseName = m_method;
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

    const QByteArray response = httpResponseHeaders(isFault, "text/xml", xmlResponse.size());
    if (m_doDebug) {
        qDebug() << "KDSoapServerSocket: writing" << response << xmlResponse;
    }
    qint64 written = write(response);
    Q_ASSERT(written == response.size()); // Please report a bug if you hit this.
    written = write(xmlResponse);
    Q_ASSERT(written == xmlResponse.size()); // Please report a bug if you hit this.
    Q_UNUSED(written);
    // flush() ?

    // All done, check if we should log this
    KDSoapServer* server = m_owner->server();
    const KDSoapServer::LogLevel logLevel = server->logLevel(); // we do this here in order to support dynamic settings changes (at the price of a mutex)
    if (logLevel != KDSoapServer::LogNothing) {
        if (logLevel == KDSoapServer::LogEveryCall ||
                (logLevel == KDSoapServer::LogFaults && isFault)) {

            if (isFault)
                server->log("FAULT " + m_method.toLatin1() + " -- " + replyMsg.faultAsString().toUtf8() + '\n');
            else
                server->log("CALL " + m_method.toLatin1() + '\n');
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

void KDSoapServerSocket::handleError(KDSoapMessage &replyMsg, const char *errorCode, const QString &error)
{
    qWarning("%s", qPrintable(error));
    replyMsg.setFault(true);
    replyMsg.addArgument(QString::fromLatin1("faultcode"), QString::fromLatin1(errorCode));
    replyMsg.addArgument(QString::fromLatin1("faultstring"), error);
}

void KDSoapServerSocket::makeCall(KDSoapServerObjectInterface* serverObjectInterface, const KDSoapMessage &requestMsg, KDSoapMessage& replyMsg, const KDSoapHeaders& requestHeaders, const QByteArray& soapAction, const QString& path)
{
    Q_ASSERT(serverObjectInterface);
    //const QString method = requestMsg.name();

    if (requestMsg.isFault()) {
        // Can this happen? Getting a fault as a request !? Doesn't make sense...
        // reply with a fault, but we don't even know what main element name to use
        // Oh well, just use the incoming fault :-)
        replyMsg = requestMsg;
        handleError(replyMsg, "Client.Data", QString::fromLatin1("Request was a fault"));
    } else {

        // Call method on m_serverObject
        serverObjectInterface->setRequestHeaders(requestHeaders, soapAction);

        KDSoapServer* server = m_owner->server();
        if (path != server->path()) {
            serverObjectInterface->processRequestWithPath(requestMsg, replyMsg, soapAction, path);
        } else {
            serverObjectInterface->processRequest(requestMsg, replyMsg, soapAction);
        }
        if (serverObjectInterface->hasFault()) {
            //qDebug() << "Got fault!";
            replyMsg.setFault(true);
            serverObjectInterface->storeFaultAttributes(replyMsg);
        }
    }
}

// Prevention against concurrent requests without waiting for a (delayed) reply,
// but untestable with QNAM on the client side, since it doesn't do that.
void KDSoapServerSocket::setSocketEnabled(bool enabled)
{
    if (m_socketEnabled == enabled)
        return;

    m_socketEnabled = enabled;
    if (enabled) {
        slotReadyRead();
    }
}

#include "moc_KDSoapServerSocket_p.cpp"
