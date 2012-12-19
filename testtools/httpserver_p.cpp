/****************************************************************************
** Copyright (C) 2010-2012 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "httpserver_p.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#ifndef QT_NO_OPENSSL
#include <QSslConfiguration>
#endif

// Helper for xmlBufferCompare
static bool textBufferCompare(
    const QByteArray& source, const QByteArray& dest,  // for the qDebug only
    QIODevice& sourceFile, QIODevice& destFile)
{
    int lineNumber = 1;
    while (!sourceFile.atEnd()) {
        if (destFile.atEnd())
            return false;
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

// A tool for comparing XML documents and outputting something useful if they differ
bool KDSoapUnitTestHelpers::xmlBufferCompare(const QByteArray& source, const QByteArray& dest)
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

void KDSoapUnitTestHelpers::httpGet(const QUrl& url)
{
    QNetworkRequest request(url);
    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.get(request);
    //QObject::connect(reply, SIGNAL(sslErrors(const QList<QSslError>&)), reply, SLOT(ignoreSslErrors()));

    QEventLoop ev;
    QObject::connect(reply, SIGNAL(finished()), &ev, SLOT(quit()));
    ev.exec();

    //QObject::connect(reply, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));
    //QTestEventLoop::instance().enterLoop(11);

    //qDebug() << "httpGet:" << reply->readAll();
    delete reply;
}

#ifndef QT_NO_OPENSSL

// To debug this:  openssl s_client -connect 127.0.0.1:443

static void setupSslServer(QSslSocket* serverSocket)
{
    Q_INIT_RESOURCE(testtools);
    //qDebug() << "setupSslServer";
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

    // Both ways work:
#if 0
    QSslConfiguration defaultConfig = QSslConfiguration::defaultConfiguration();
    QFile certFile(QString::fromLatin1(":/certs/cacert.pem"));
    if (!certFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open cacert.pem";
        return false;
    }
    QSslCertificate cert(&certFile);
    if (!cert.isValid())
        return false;
    defaultConfig.setCaCertificates(QList<QSslCertificate>() << cert);
    QSslConfiguration::setDefaultConfiguration(defaultConfig);
#endif

    QFile certFile(QString::fromLatin1(":/certs/cacert.pem"));
    if (!certFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open cacert.pem";
        return false;
    }
    QSslCertificate cert(&certFile);
    const QDateTime currentTime = QDateTime::currentDateTime();
    if (cert.effectiveDate() > currentTime
            || cert.expiryDate() < currentTime) {
        qDebug() << "Certificate" << certFile.fileName() << "is not valid";
        qDebug() << "It is valid from" << cert.effectiveDate() << "to" << cert.expiryDate();
        return false;
    }
    QSslSocket::addDefaultCaCertificate(cert);

    return true;
}
#endif

// A blocking http server (must be used in a thread) which supports SSL.
class BlockingHttpServer : public QTcpServer
{
    Q_OBJECT
public:
    BlockingHttpServer(bool ssl) : doSsl(ssl), sslSocket(0) {}
    ~BlockingHttpServer() {}

    QTcpSocket* waitForNextConnectionSocket() {
        if (!waitForNewConnection(10000)) // 2000 would be enough, except in valgrind
            return 0;
        if (doSsl) {
            Q_ASSERT(sslSocket);
            return sslSocket;
        } else {
            //qDebug() << "returning nextPendingConnection";
            return nextPendingConnection();
        }
    }

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    virtual void incomingConnection(qintptr socketDescriptor)
#else
    virtual void incomingConnection(int socketDescriptor)
#endif
    {
#ifndef QT_NO_OPENSSL
        if (doSsl) {
            QSslSocket *serverSocket = new QSslSocket;
            serverSocket->setParent(this);
            serverSocket->setSocketDescriptor(socketDescriptor);
            connect(serverSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotSslErrors(QList<QSslError>)));
            setupSslServer(serverSocket);
            //qDebug() << "Created QSslSocket, starting server encryption";
            serverSocket->startServerEncryption();
            sslSocket = serverSocket;
            // If startServerEncryption fails internally [and waitForEncrypted hangs],
            // then this is how to debug it.
            // A way to catch such errors is really missing in Qt..
            //qDebug() << "startServerEncryption said:" << sslSocket->errorString();
            bool ok = serverSocket->waitForEncrypted();
            Q_ASSERT(ok);
            Q_UNUSED(ok);
        } else
#endif
            QTcpServer::incomingConnection(socketDescriptor);
    }
    void disableSsl() { doSsl = false; }
private slots:
#ifndef QT_NO_OPENSSL
    void slotSslErrors(const QList<QSslError>& errors)
    {
        qDebug() << "slotSslErrors" << sslSocket->errorString() << errors;
    }
#endif
private:
    bool doSsl;
    QTcpSocket* sslSocket;
};

static bool splitHeadersAndData(const QByteArray& request, QByteArray& header, QByteArray& data)
{
    const int sep = request.indexOf("\r\n\r\n");
    if (sep <= 0)
        return false;
    header = request.left(sep);
    data = request.mid(sep + 4);
    return true;
}

typedef QMap<QByteArray, QByteArray> HeadersMap;
static HeadersMap parseHeaders(const QByteArray& headerData) {
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
        if (pos == -1)
            qDebug() << "Malformed HTTP header:" << line;
        const QByteArray header = line.left(pos);
        const QByteArray value = line.mid(pos+1).trimmed(); // remove space before and \r\n after
        //qDebug() << "HEADER" << header << "VALUE" << value;
        headersMap.insert(header, value);
    }
    return headersMap;
}


void HttpServerThread::disableSsl()
{
    m_server->disableSsl();
}

void HttpServerThread::run()
{
    m_server = new BlockingHttpServer(m_features & Ssl);
    m_server->listen();
    m_port = m_server->serverPort();
    m_ready.release();

    const bool doDebug = qgetenv("KDSOAP_DEBUG").toInt();

    if (doDebug)
        qDebug() << "HttpServerThread listening on port" << m_port;

    // Wait for first connection (we'll wait for further ones inside the loop)
    QTcpSocket *clientSocket = m_server->waitForNextConnectionSocket();
    Q_ASSERT(clientSocket);

    Q_FOREVER {
        // get the "request" packet
        if (doDebug) {
            qDebug() << "HttpServerThread: waiting for read";
        }
        if (clientSocket->state() == QAbstractSocket::UnconnectedState ||
            !clientSocket->waitForReadyRead(2000)) {
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
        const bool splitOK = splitHeadersAndData(request, m_receivedHeaders, m_receivedData);
        if (!splitOK) {
            //if (doDebug)
            //    qDebug() << "Storing partial request" << request;
            m_partialRequest = request;
            continue;
        }

        m_headers = parseHeaders(m_receivedHeaders);

        if (m_headers.value("Content-Length").toInt() > m_receivedData.size()) {
            //if (doDebug)
            //    qDebug() << "Storing partial request" << request;
            m_partialRequest = request;
            continue;
        }

        m_partialRequest.clear();

        if (m_headers.value("_path").endsWith("terminateThread")) // we're asked to exit
            break; // normal exit

        // TODO compared with expected SoapAction
        QList<QByteArray> contentTypes = m_headers.value("Content-Type").split(';');
        if (contentTypes[0] == "text/xml" && m_headers.value("SoapAction").isEmpty()) {
            qDebug() << "ERROR: no SoapAction set for Soap 1.1";
            break;
        }else if( contentTypes[0] == "application/soap+xml" && !contentTypes[2].startsWith("action")){
            qDebug() << "ERROR: no SoapAction set for Soap 1.2";
            break;
        }

        //qDebug() << "headers received:" << m_receivedHeaders;
        //qDebug() << headers;
        //qDebug() << "data received:" << m_receivedData;


        if (m_features & BasicAuth) {
            QByteArray authValue = m_headers.value("Authorization");
            if (authValue.isEmpty())
                authValue = m_headers.value("authorization"); // as sent by Qt-4.5
            bool authOk = false;
            if (!authValue.isEmpty()) {
                //qDebug() << "got authValue=" << authValue; // looks like "Basic <base64 of user:pass>"
                Method method;
                QString headerVal;
                parseAuthLine(QString::fromLatin1(authValue.data(),authValue.size()), &method, &headerVal);
                //qDebug() << "method=" << method << "headerVal=" << headerVal;
                switch (method) {
                case None: // we want auth, so reject "None"
                    break;
                case Basic:
                    {
                        const QByteArray userPass = QByteArray::fromBase64(headerVal.toLatin1());
                        //qDebug() << userPass;
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
                const QByteArray unauthorized = "HTTP/1.1 401 Authorization Required\r\nWWW-Authenticate: Basic realm=\"example\"\r\nContent-Length: 0\r\n\r\n";
                clientSocket->write( unauthorized );
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

#include "moc_httpserver_p.cpp"
#include "httpserver_p.moc"
