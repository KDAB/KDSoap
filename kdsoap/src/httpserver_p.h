#ifndef HTTPSERVER_P_H
#define HTTPSERVER_P_H

#include "KDSoapGlobal.h"
#include <QBuffer>
#include <QThread>
#include <QSemaphore>
#include <QTcpServer>
#include <QTcpSocket>
#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#endif
#include <QUrl>

namespace KDSoapUnitTestHelpers
{
    KDSOAP_EXPORT bool xmlBufferCompare(const QByteArray& source, const QByteArray& dest);
    KDSOAP_EXPORT QByteArray makeHttpResponse(const QByteArray& responseData);
    KDSOAP_EXPORT void httpGet(const QUrl& url);
}

#ifndef QT_NO_OPENSSL
static void setupSslServer(QSslSocket* serverSocket)
{
    serverSocket->setProtocol(QSsl::AnyProtocol);
    // TODO
    //serverSocket->setLocalCertificate (SRCDIR "/certs/server.pem");
    //serverSocket->setPrivateKey (SRCDIR  "/certs/server.key");
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
        if (!waitForNewConnection(2000))
            return 0;
        if (doSsl) {
            Q_ASSERT(sslSocket);
            return sslSocket;
        } else {
            //qDebug() << "returning nextPendingConnection";
            return nextPendingConnection();
        }
    }
    virtual void incomingConnection(int socketDescriptor)
    {
#ifndef QT_NO_OPENSSL
        if (doSsl) {
            QSslSocket *serverSocket = new QSslSocket;
            serverSocket->setParent(this);
            serverSocket->setSocketDescriptor(socketDescriptor);
            connect(serverSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotSslErrors(QList<QSslError>)));
            setupSslServer(serverSocket);
            serverSocket->startServerEncryption();
            sslSocket = serverSocket;
        } else
#endif
            QTcpServer::incomingConnection(socketDescriptor);
    }
private slots:
#ifndef QT_NO_OPENSSL
    void slotSslErrors(const QList<QSslError>& errors)
    {
        qDebug() << "slotSslErrors" << sslSocket->errorString() << errors;
    }
#endif
private:
    const bool doSsl;
    QTcpSocket* sslSocket;
};

class KDSOAP_EXPORT HttpServerThread : public QThread
{
    Q_OBJECT
public:
    enum Feature {
        Public = 0,    // HTTP with no ssl and no authentication needed
        Ssl = 1,       // HTTPS
        BasicAuth = 2  // Requires authentication
        // bitfield, next item is 4!
    };
    Q_DECLARE_FLAGS(Features, Feature)

    HttpServerThread(const QByteArray& dataToSend, Features features)
        : m_dataToSend(dataToSend), m_features(features)
    {
        start();
        m_ready.acquire();

    }
    ~HttpServerThread()
    {
        finish();
        wait();
    }

    inline int serverPort() const { return m_port; }
    QString endPoint() const {
        return QString::fromLatin1("%1://127.0.0.1:%2/path")
                           .arg(QString::fromLatin1((m_features & Ssl)?"https":"http"))
                           .arg(serverPort());
    }

    inline void finish() {
        KDSoapUnitTestHelpers::httpGet(endPoint() + QLatin1String("/terminateThread"));
    }

    QByteArray receivedData() const { return m_receivedData; }
    QByteArray receivedHeaders() const { return m_receivedHeaders; }
    void resetReceivedBuffers() {
        m_receivedData.clear();
        m_receivedHeaders.clear();
    }

protected:
    /* \reimp */ void run()
    {
        BlockingHttpServer server(m_features & Ssl);
        server.listen();
        m_port = server.serverPort();
        m_ready.release();

        const bool doDebug = qgetenv("KDSOAP_DEBUG").toInt();

        if (doDebug)
            qDebug() << "HttpServerThread listening on port" << m_port;

        // Wait for first connection (we'll wait for further ones inside the loop)
        QTcpSocket *clientSocket = server.waitForNextConnectionSocket();

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
                    clientSocket = server.waitForNextConnectionSocket();
                    Q_ASSERT(clientSocket);
                    continue; // go to "waitForReadyRead"
                } else {
                    qDebug() << "HttpServerThread:" << clientSocket->error() << "waiting for \"request\" packet";
                    break;
                }
            }
            const QByteArray request = clientSocket->readAll();
            if (doDebug) {
                qDebug() << "HttpServerThread: request:" << request;
            }
            // Split headers and request xml
            const bool splitOK = splitHeadersAndData(request, m_receivedHeaders, m_receivedData);
            Q_ASSERT(splitOK);
            Q_UNUSED(splitOK); // To avoid a warning if Q_ASSERT doesn't expand to anything.
            QMap<QByteArray, QByteArray> headers = parseHeaders(m_receivedHeaders);

            if (headers.value("_path").endsWith("terminateThread")) // we're asked to exit
                break; // normal exit
            // TODO compared with expected SoapAction
            if (headers.value("SoapAction").isEmpty()) {
                qDebug() << "ERROR: no SoapAction set";
                break;
            }

            //qDebug() << "headers received:" << m_receivedHeader;
            //qDebug() << "data received:" << m_receivedData;

            if (m_features & BasicAuth) {
                const QByteArray authValue = headers.value("Authorization");
                bool authOk = false;
                if (!authValue.isEmpty()) {
                    //qDebug() << "got authValue=" << authValue; // looks like "Basic <base64 of user:pass>"
                    Method method;
                    QString headerVal;
                    parseAuthLine(QString::fromLatin1(authValue), &method, &headerVal);
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
        if (doDebug) {
            qDebug() << "HttpServerThread terminated";
        }
    }

private:
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
    HeadersMap parseHeaders(const QByteArray& headerData) const{
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

    enum Method { None, Basic, Plain, Login, Ntlm, CramMd5, DigestMd5 };
    static void parseAuthLine(const QString& str, Method* method, QString* headerVal)
    {
        *method = None;
        // The code below (from QAuthenticatorPrivate::parseHttpResponse)
        // is supposed to be run in a loop, apparently
        // (multiple WWW-Authenticate lines? multiple values in the line?)

        //qDebug() << "parseAuthLine() " << str;
        if (*method < Basic && str.startsWith(QLatin1String("Basic"), Qt::CaseInsensitive)) {
            *method = Basic;
            *headerVal = str.mid(6);
        } else if (*method < Ntlm && str.startsWith(QLatin1String("NTLM"), Qt::CaseInsensitive)) {
            *method = Ntlm;
            *headerVal = str.mid(5);
        } else if (*method < DigestMd5 && str.startsWith(QLatin1String("Digest"), Qt::CaseInsensitive)) {
            *method = DigestMd5;
            *headerVal = str.mid(7);
        }
    }

    static QByteArray makeHttpResponse(const QByteArray& responseData)
    {
        QByteArray httpResponse("HTTP/1.1 200 OK\r\nContent-Type: text/xml\r\nContent-Length: ");
        httpResponse += QByteArray::number(responseData.size());
        httpResponse += "\r\n";

        // We don't support multiple connexions (TODO) so let's ask the client
        // to close the connection every time. See testCallNoReply which performs
        // multiple connexions at the same time (QNAM keeps the old connexion open).
        httpResponse += "Connection: close\r\n";
        httpResponse += "\r\n";
        httpResponse += responseData;
        return httpResponse;
    }

private:
    QSemaphore m_ready;
    QByteArray m_dataToSend;
    QByteArray m_receivedData;
    QByteArray m_receivedHeaders;
    int m_port;
    Features m_features;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HttpServerThread::Features)

#endif /* HTTPSERVER_P_H */

