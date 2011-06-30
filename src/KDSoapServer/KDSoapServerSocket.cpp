#include "KDSoapServerSocket_p.h"
#include "KDSoapSocketList_p.h"
#include "KDSoapServerObjectInterface.h"
#include "KDSoapServer.h"
#include <KDSoapMessage.h>
#include <KDSoapNamespaceManager.h>
#include <KDSoapMessageWriter_p.h>
#include <QBuffer>
#include <QThread>
#include <QMetaMethod>
#include <QFile>
#include <QFileInfo>
#include <QVarLengthArray>

KDSoapServerSocket::KDSoapServerSocket(KDSoapSocketList* owner, QObject* serverObject)
#ifndef QT_NO_OPENSSL
    : QSslSocket(),
#else
    : QTcpSocket(),
#endif
      m_owner(owner),
      m_serverObject(serverObject)
{
    connect(this, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
    m_doDebug = qgetenv("KDSOAP_DEBUG").toInt();
}

// The socket is deleted when it emits disconnected() (see KDSoapSocketList::handleIncomingConnection).
KDSoapServerSocket::~KDSoapServerSocket()
{
    m_owner->socketDeleted(this);
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
    const QByteArray request = firstLine.at(0);
    const QByteArray path = firstLine.at(1);
    const QByteArray httpVersion = firstLine.at(2);
    if (request != "GET" && request != "POST") {
        qDebug() << "Unknown HTTP request:" << firstLine;
        return headersMap;
    }
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
    //qDebug() << QThread::currentThread() << "slotReadyRead!";
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

    KDSoapServer* server = m_owner->server();
    KDSoapMessage replyMsg;
    replyMsg.setUse(server->use());

    const QString path = QString::fromLatin1(httpHeaders.value("_path").constData());
    if (path != server->path()) {
        m_requestBuffer.clear();
        if (path == server->wsdlPathInUrl()) {
            const QString wsdlFile = server->wsdlFile();
            QFile wf(wsdlFile);
            if (wf.open(QIODevice::ReadOnly)) {
                qDebug() << "Returning wsdl file contents";
                const QByteArray responseText = wf.readAll();
                const QByteArray response = httpResponseHeaders(false, "text/plain", responseText.size());
                write(response);
                write(responseText);
                return;
            }
        }
        handleError(replyMsg, "Client.Data", QString::fromLatin1("Invalid path '%1'").arg(path));
    }

    //parse message
    KDSoapMessage requestMsg;
    QString messageNamespace;
    KDSoapHeaders requestHeaders;
    KDSoapMessage::XmlError err = requestMsg.parseSoapXml(receivedData, &messageNamespace, &requestHeaders);
    if (err == KDSoapMessage::PrematureEndOfDocumentError) {
        //qDebug() << "Incomplete SOAP message, wait for more data";
        //incomplete request, wait for more data
        return;
    } //TODO handle parse errors?

    m_requestBuffer.clear();

    // check soap version and extract soapAction header
    QByteArray soapAction;
    const QByteArray contentType = httpHeaders.value("content-type");
    if (contentType.startsWith("text/xml")) {
        // SOAP 1.1
        soapAction = httpHeaders.value("soapaction");
        // The SOAP standard allows quotation marks around the SoapAction, so we have to get rid of these.
        if (soapAction.startsWith('\"'))
            soapAction = soapAction.mid(1, soapAction.length() - 2);

    } else if (contentType.startsWith("application/soap+xml")) {
        // SOAP 1.2
        // Example: application/soap+xml;charset=utf-8;action=ActionHex
        const QList<QByteArray> parts = contentType.split(';');
        Q_FOREACH(const QByteArray& part, parts) {
            if (part.startsWith("action=")) {
                soapAction = part.mid(strlen("action="));
            }
        }
    }

    const QString method = requestMsg.name();

    KDSoapHeaders responseHeaders;
    if (!replyMsg.isFault()) {
        makeCall(requestMsg, replyMsg, requestHeaders, responseHeaders, soapAction);
    }

    const bool isFault = replyMsg.isFault();

    // send replyMsg on socket

    KDSoapMessageWriter msgWriter;
    msgWriter.setMessageNamespace(messageNamespace);
    // Note that the kdsoap client parsing code doesn't care for the name (except if it's fault), even in
    // Document mode. Other implementations do, though.
    const QString responseName = isFault ? QString::fromLatin1("Fault") : method;
    const QByteArray xmlResponse = msgWriter.messageToXml(replyMsg, responseName, responseHeaders, QMap<QString, KDSoapMessage>());
    const QByteArray response = httpResponseHeaders(isFault, "text/xml", xmlResponse.size());
    if (m_doDebug) {
        qDebug() << "HttpServerThread: writing" << response << xmlResponse;
    }
    write(response);
    write(xmlResponse);
    // flush() ?

    // All done, check if we should log this
    const KDSoapServer::LogLevel logLevel = server->logLevel(); // we do this here in order to support dynamic settings changes (at the price of a mutex)
    if (logLevel != KDSoapServer::LogNothing) {
        if (logLevel == KDSoapServer::LogEveryCall ||
                (logLevel == KDSoapServer::LogFaults && isFault)) {

            if (isFault)
                server->log("FAULT " + method.toLatin1() + " -- " + replyMsg.faultAsString().toUtf8() + '\n');
            else
                server->log("CALL " + method.toLatin1() + '\n');
        }
    }
}

void KDSoapServerSocket::handleError(KDSoapMessage &replyMsg, const char *errorCode, const QString &error)
{
    qWarning("%s", qPrintable(error));
    replyMsg.setFault(true);
    replyMsg.addArgument(QString::fromLatin1("faultcode"), QString::fromLatin1(errorCode));
    replyMsg.addArgument(QString::fromLatin1("faultstring"), error);
}

void KDSoapServerSocket::makeCall(const KDSoapMessage &requestMsg, KDSoapMessage& replyMsg, const KDSoapHeaders& requestHeaders, KDSoapHeaders& responseHeaders, const QByteArray& soapAction)
{
    //const QString method = requestMsg.name();

    if (requestMsg.isFault()) {
        // Can this happen? Getting a fault as a request !? Doesn't make sense...
        // reply with a fault, but we don't even know what main element name to use
        // Oh well, just use the incoming fault :-)
        replyMsg = requestMsg;
        handleError(replyMsg, "Client.Data", QString::fromLatin1("Request was a fault"));
    } else {
        // Call method on m_serverObject
        KDSoapServerObjectInterface* serverObjectInterface = qobject_cast<KDSoapServerObjectInterface *>(m_serverObject);
        if (!serverObjectInterface) {
            const QString error = QString::fromLatin1("Server object %1 does not implement KDSoapServerObjectInterface!").arg(QString::fromLatin1(m_serverObject->metaObject()->className()));
            handleError(replyMsg, "Server.ImplementationError", error);
            return;
        }

        serverObjectInterface->setRequestHeaders(requestHeaders, soapAction);

        serverObjectInterface->processRequest(requestMsg, replyMsg, soapAction);

        responseHeaders = serverObjectInterface->responseHeaders();

        if (serverObjectInterface->hasFault()) {
            //qDebug() << "Got fault!";
            replyMsg.setFault(true);
            serverObjectInterface->storeFaultAttributes(replyMsg);
        }
    }
}

#include "moc_KDSoapServerSocket_p.cpp"
