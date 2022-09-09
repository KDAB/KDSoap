/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "httpserver_p.h"
#include <QDateTime>
#include <QDomDocument>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#ifndef QT_NO_OPENSSL
#include <QSslConfiguration>
#endif

// Helper for xmlBufferCompare
static bool textBufferCompare(const QByteArray &source, const QByteArray &dest, // for the qDebug only
                              QIODevice &sourceFile, QIODevice &destFile)
{
    int lineNumber = 1;
    while (!sourceFile.atEnd()) {
        if (destFile.atEnd()) {
            return false;
        }
        QByteArray sourceLine = sourceFile.readLine();
        QByteArray destLine = destFile.readLine();
        if (sourceLine != destLine) {
            sourceLine.chop(1); // remove '\n'
            destLine.chop(1); // remove '\n'
            qDebug() << source << "and" << dest << "differ at line" << lineNumber;
            qDebug("got     : %s", sourceLine.constData());
            qDebug("expected: %s", destLine.constData());
            return false;
        }
        ++lineNumber;
    }
    return true;
}

static void initHashSeed()
{
    qSetGlobalQHashSeed(0);
}

Q_CONSTRUCTOR_FUNCTION(initHashSeed)

// A tool for comparing XML documents and outputting something useful if they differ
bool KDSoapUnitTestHelpers::xmlBufferCompare(const QByteArray &source, const QByteArray &dest)
{
    QBuffer sourceFile;
    sourceFile.setData(source);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR opening QIODevice";
        return false;
    }
    QBuffer destFile;
    destFile.setData(dest);
    if (!destFile.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR opening QIODevice";
        return false;
    }

    // Use QDomDocument to reformat the XML with newlines
    QDomDocument sourceDoc;
    if (!sourceDoc.setContent(&sourceFile)) {
        qDebug() << "ERROR parsing XML:" << source;
        return false;
    }
    QDomDocument destDoc;
    if (!destDoc.setContent(&destFile)) {
        qDebug() << "ERROR parsing XML:" << dest;
        return false;
    }

    const QByteArray sourceXml = sourceDoc.toByteArray();
    const QByteArray destXml = destDoc.toByteArray();
    sourceFile.close();
    destFile.close();

    QBuffer sourceBuffer;
    sourceBuffer.setData(sourceXml);
    sourceBuffer.open(QIODevice::ReadOnly);
    QBuffer destBuffer;
    destBuffer.setData(destXml);
    destBuffer.open(QIODevice::ReadOnly);

    return textBufferCompare(source, dest, sourceBuffer, destBuffer);
}

void KDSoapUnitTestHelpers::httpGet(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);
    // QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));

    QEventLoop ev;
    QObject::connect(reply, &QNetworkReply::finished, &ev, &QEventLoop::quit);
    ev.exec();

    // QObject::connect(reply, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));
    // QTestEventLoop::instance().enterLoop(11);

    // qDebug() << "httpGet:" << reply->readAll();
    delete reply;
}

#ifndef QT_NO_OPENSSL

// To debug this:  openssl s_client -connect 127.0.0.1:443

static void setupSslServer(QSslSocket *serverSocket)
{
    Q_INIT_RESOURCE(testtools);
    // qDebug() << "setupSslServer";
    serverSocket->setProtocol(QSsl::AnyProtocol);
    serverSocket->setLocalCertificate(QString::fromLatin1(":/certs/test-127.0.0.1-cert.pem"));
    serverSocket->setPrivateKey(QString::fromLatin1(":/certs/test-127.0.0.1-key.pem"));
}

static void initResource()
{
    Q_INIT_RESOURCE(testtools);
}

bool KDSoapUnitTestHelpers::setSslConfiguration()
{
    initResource();

    // To make SSL work, we need to tell Qt about our local certificate

    QSslConfiguration defaultConfig = QSslConfiguration::defaultConfiguration();
    QFile certFile(QString::fromLatin1(":/certs/cacert.pem"));
    if (!certFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open cacert.pem";
        return false;
    }
    QSslCertificate cert(&certFile);
    const QDateTime currentTime = QDateTime::currentDateTime();
    if (cert.effectiveDate() > currentTime || cert.expiryDate() < currentTime) {
        qDebug() << "Certificate" << certFile.fileName() << "is not valid";
        qDebug() << "It is valid from" << cert.effectiveDate() << "to" << cert.expiryDate();
        return false;
    }
    defaultConfig.setCaCertificates({cert});
    QSslConfiguration::setDefaultConfiguration(defaultConfig);

    return true;
}
#endif

// A blocking http server (must be used in a thread) which supports SSL.
class BlockingHttpServer : public QTcpServer
{
    Q_OBJECT
public:
    BlockingHttpServer(bool ssl)
        : doSsl(ssl)
        , sslSocket(0)
    {
    }
    ~BlockingHttpServer()
    {
    }

    QTcpSocket *waitForNextConnectionSocket()
    {
        if (!waitForNewConnection(20000)) { // 2000 would be enough, except in valgrind
            return 0;
        }
        return nextPendingConnection();
    }

    virtual void incomingConnection(qintptr socketDescriptor) override
    {
#ifndef QT_NO_OPENSSL
        if (doSsl) {
            QSslSocket *serverSocket = new QSslSocket;
            serverSocket->setParent(this);
            serverSocket->setSocketDescriptor(socketDescriptor);
            connect(serverSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &BlockingHttpServer::slotSslErrors);
            setupSslServer(serverSocket);
            // qDebug() << "Created QSslSocket, starting server encryption";
            serverSocket->startServerEncryption();
            sslSocket = serverSocket;
            // If startServerEncryption fails internally,
            // then this is how to debug it.
            // A way to catch such errors is really missing in Qt..
            // qDebug() << "startServerEncryption said:" << sslSocket->errorString();
            serverSocket->waitForEncrypted();
            addPendingConnection(serverSocket);
        } else
#endif
            QTcpServer::incomingConnection(socketDescriptor);
    }
    void disableSsl()
    {
        doSsl = false;
    }

private slots:
    void slotSslErrors(const QList<QSslError> &errors)
    {
#ifndef QT_NO_OPENSSL
        qDebug() << "server-side: slotSslErrors" << sslSocket->errorString() << errors;
#else
        Q_UNUSED(errors);
#endif
    }

private:
    bool doSsl;
    QTcpSocket *sslSocket;
};

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
    const QByteArray request = firstLine[0];
    const QByteArray path = firstLine[1];
    const QByteArray httpVersion = firstLine[2];
    if (request != "GET" && request != "POST") {
        qDebug() << "Unknown HTTP request:" << firstLine;
        return headersMap;
    }
    headersMap.insert("_path", path);
    headersMap.insert("_httpVersion", httpVersion);

    while (!sourceBuffer.atEnd()) {
        const QByteArray line = sourceBuffer.readLine();
        const int pos = line.indexOf(':');
        if (pos == -1) {
            qDebug() << "Malformed HTTP header:" << line;
        }
        const QByteArray header = line.left(pos);
        const QByteArray value = line.mid(pos + 1).trimmed(); // remove space before and \r\n after
        // qDebug() << "HEADER" << header << "VALUE" << value;
        headersMap.insert(header, value);
    }
    return headersMap;
}

void HttpServerThread::disableSsl()
{
    if (m_server) {
        m_server->disableSsl();
    }
}

void HttpServerThread::run()
{
    m_server = new BlockingHttpServer(m_features & Ssl);
    if (!m_server->listen()) {
        qFatal("HttpServerThread::run is unable to listen");
    }
    QMutexLocker lock(&m_mutex);
    m_port = m_server->serverPort();
    lock.unlock();
    m_ready.release();

    const bool doDebug = qEnvironmentVariableIsSet("KDSOAP_DEBUG");

    if (doDebug) {
        qDebug() << "HttpServerThread listening on port" << m_port;
    }

    // Wait for first connection (we'll wait for further ones inside the loop)
    QTcpSocket *clientSocket = m_server->waitForNextConnectionSocket();
    Q_ASSERT(clientSocket);
    if (!clientSocket) {
        qWarning() << "No client connected to this server. Fatal error";
        delete m_server;
        return;
    }

    Q_FOREVER
    {
        // get the "request" packet
        if (doDebug) {
            qDebug() << "HttpServerThread: waiting for read";
        }
        if (clientSocket->state() == QAbstractSocket::UnconnectedState || !clientSocket->waitForReadyRead(2000)) {
            if (clientSocket->state() == QAbstractSocket::UnconnectedState) {
                delete clientSocket;
                if (doDebug) {
                    qDebug() << "Waiting for next connection...";
                }
                clientSocket = m_server->waitForNextConnectionSocket();
                Q_ASSERT(clientSocket);
                continue; // go to "waitForReadyRead"
            } else {
                qDebug() << "HttpServerThread:" << clientSocket->error() << "waiting for \"request\" packet";
                break;
            }
        }
        const QByteArray request = m_partialRequest + clientSocket->readAll();
        if (doDebug) {
            qDebug() << "HttpServerThread: request:" << request;
        }

        // Split headers and request xml
        lock.relock();
        const bool splitOK = splitHeadersAndData(request, m_receivedHeaders, m_receivedData);
        if (!splitOK) {
            // if (doDebug)
            //    qDebug() << "Storing partial request" << request;
            m_partialRequest = request;
            continue;
        }

        m_headers = parseHeaders(m_receivedHeaders);

        if (m_headers.value("Content-Length").toInt() > m_receivedData.size()) {
            // if (doDebug)
            //    qDebug() << "Storing partial request" << request;
            m_partialRequest = request;
            continue;
        }

        m_partialRequest.clear();

        if (m_headers.value("_path").endsWith("terminateThread")) { // we're asked to exit
            break; // normal exit
        }

        QList<QByteArray> contentTypes = m_headers.value("Content-Type").split(';');
        if (contentTypes[0] == "text/xml") {
            if (m_headers.value("SoapAction").isEmpty()) {
                qDebug() << "ERROR: no SoapAction set for Soap 1.1";
                break;
            }

            if (!m_expectedSoapAction.isEmpty() && m_headers.value("SoapAction") != m_expectedSoapAction) {
                qDebug("ERROR: Client sent SoapAction HTTP header (\"%s\") which does not match the expected (\"%s\")",
                       m_headers.value("SoapAction").constData(), m_expectedSoapAction.constData());
                break;
            }
        } else if (m_clientSendsActionInHttpHeader
                   && contentTypes[0] == "application/soap+xml") {
            if (!contentTypes[2].startsWith("action")) {
                qDebug() << "ERROR: no SoapAction set for Soap 1.2";
                break;
            }

            QList<QByteArray> actionParts = contentTypes[2].split('=');
            if (actionParts.length() < 2) {
                qDebug("ERROR: The action parameter is malformed in the HTTP Content-type header: \"%s\"",
                       contentTypes[2].constData());
                break;
            }

            if (!m_expectedSoapAction.isEmpty()) {
                actionParts = actionParts[1].split(';');
                if (actionParts[0] != m_expectedSoapAction) {
                    qDebug("ERROR: The 'action' parameter which was sent in the HTTP Content-type header (\"%s\") "
                           "does not match the expected SOAP action: \"%s\"",
                           m_headers.value("SoapAction").constData(), m_expectedSoapAction.constData());
                    break;
                }
            }
        }
        lock.unlock();

        // qDebug() << "headers received:" << m_receivedHeaders;
        // qDebug() << headers;
        // qDebug() << "data received:" << m_receivedData;

        if (m_features & BasicAuth) {
            QByteArray authValue = m_headers.value("Authorization");
            if (authValue.isEmpty()) {
                authValue = m_headers.value("authorization"); // as sent by Qt-4.5
            }
            bool authOk = false;
            if (!authValue.isEmpty()) {
                // qDebug() << "got authValue=" << authValue; // looks like "Basic <base64 of user:pass>"
                Method method;
                QString headerVal;
                parseAuthLine(QString::fromLatin1(authValue.data(), authValue.size()), &method, &headerVal);
                // qDebug() << "method=" << method << "headerVal=" << headerVal;
                switch (method) {
                case None: // we want auth, so reject "None"
                    break;
                case Basic: {
                    const QByteArray userPass = QByteArray::fromBase64(headerVal.toLatin1());
                    // qDebug() << userPass;
                    // TODO if (validateAuth(userPass)) {
                    if (userPass == ("kdab:testpass")) {
                        authOk = true;
                    }
                    break;
                }
                default:
                    qWarning("Unsupported authentication mechanism %s", authValue.constData());
                }
            }

            if (!authOk) {
                // send auth request (Qt supports basic, ntlm and digest)
                const QByteArray unauthorized =
                    "HTTP/1.1 401 Authorization Required\r\nWWW-Authenticate: Basic realm=\"example\"\r\nContent-Length: 0\r\n\r\n";
                clientSocket->write(unauthorized);
                if (!clientSocket->waitForBytesWritten(2000)) {
                    qDebug() << "HttpServerThread:" << clientSocket->error() << "writing auth request";
                    break;
                }
                continue;
            }
        }

        // send response
        QByteArray response = makeHttpResponse(m_dataToSend);
        if (doDebug) {
            qDebug() << "HttpServerThread: writing" << response;
        }
        clientSocket->write(response);

        clientSocket->flush();
    }
    // all done...
    delete clientSocket;
    delete m_server;
    if (doDebug) {
        qDebug() << "HttpServerThread terminated";
    }
}

QByteArray HttpServerThread::expectedSoapAction() const
{
    return m_expectedSoapAction;
}

void HttpServerThread::setExpectedSoapAction(const QByteArray &expectedSoapAction)
{
    m_expectedSoapAction = expectedSoapAction;
}

bool HttpServerThread::clientSendsActionInHttpHeader() const
{
    return m_clientSendsActionInHttpHeader;
}

void HttpServerThread::setClientSendsActionInHttpHeader(bool clientUseWSAddressing)
{
    m_clientSendsActionInHttpHeader = clientUseWSAddressing;
}

const char *KDSoapUnitTestHelpers::xmlEnvBegin11()
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
           "<soap:Envelope"
           " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
           " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
           " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
           " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
}

const char *KDSoapUnitTestHelpers::xmlEnvBegin11WithWSAddressing()
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
           "<soap:Envelope"
           " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
           " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
           " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
           " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
           " xmlns:wsa=\"http://www.w3.org/2005/08/addressing\"";
}


const char *KDSoapUnitTestHelpers::xmlEnvBegin12()
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
           "<soap:Envelope"
           " xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\""
           " xmlns:soap-enc=\"http://www.w3.org/2003/05/soap-encoding\""
           " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
           " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
}

const char *KDSoapUnitTestHelpers::xmlEnvBegin12WithWSAddressing()
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
           "<soap:Envelope"
           " xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\""
           " xmlns:soap-enc=\"http://www.w3.org/2003/05/soap-encoding\""
           " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
           " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
           " xmlns:wsa=\"http://www.w3.org/2005/08/addressing\""
           " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\"";
}

const char *KDSoapUnitTestHelpers::xmlEnvEnd()
{
    return "</soap:Envelope>";
}

#include "httpserver_p.moc"
#include "moc_httpserver_p.cpp"
