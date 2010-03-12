#ifndef HTTPSERVER_P_H
#define HTTPSERVER_P_H

#include <QBuffer>
#include <QThread>
#include <QSemaphore>
#include <QTcpServer>
#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#endif
#include <QUrl>

namespace KDSoapUnitTestHelpers
{
    bool xmlBufferCompare(const QByteArray& source, const QByteArray& dest);
    QByteArray makeHttpResponse(const QByteArray& responseData);
    void httpGet(const QUrl& url);
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

class HttpServerThread : public QThread
{
    Q_OBJECT
public:
    HttpServerThread(const QByteArray& dataToSend, bool ssl)
        : m_dataToSend(dataToSend), m_ssl(ssl)
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
                           .arg(QString::fromLatin1(m_ssl?"https":"http"))
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
        BlockingHttpServer server(m_ssl);
        server.listen();
        m_port = server.serverPort();
        m_ready.release();

        //qDebug() << "HttpServerThread listening on port" << m_port;

        QTcpSocket *clientSocket = server.waitForNextConnectionSocket();
        Q_FOREVER {
            // get the "request" packet
            if (!clientSocket->waitForReadyRead(2000)) {
                if (clientSocket->state() == QAbstractSocket::UnconnectedState) {
                    delete clientSocket;
                    //qDebug() << "Waiting for next connection...";
                    clientSocket = server.waitForNextConnectionSocket();
                    Q_ASSERT(clientSocket);
                    continue; // go to "waitForReadyRead"
                } else {
                    qDebug() << "HttpServerThread:" << clientSocket->error() << "waiting for \"request\" packet";
                    break;
                }
            }
            const QByteArray request = clientSocket->readAll();
            //qDebug() << request;
            // Split headers and request xml
            const int sep = request.indexOf("\r\n\r\n");
            Q_ASSERT(sep > 0);
            m_receivedHeaders = request.left(sep);
            QMap<QByteArray, QByteArray> headers = parseHeaders(m_receivedHeaders);
            if (headers.value("_path").endsWith("terminateThread")) // we're asked to exit
                break; // normal exit
            if (headers.value("SoapAction").isEmpty()) {
                qDebug() << "ERROR: no SoapAction set";
                break;
            }
            m_receivedData = request.mid(sep + 4);

            //qDebug() << "headers received:" << m_receivedHeader;
            //qDebug() << "data received:" << m_receivedData;

            // send response
            //qDebug() << "writing" << m_dataToSend;
            clientSocket->write( m_dataToSend );
            clientSocket->flush();
        }
        // all done...
        delete clientSocket;
    }

private:
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

private:
    QSemaphore m_ready;
    QByteArray m_dataToSend;
    QByteArray m_receivedData;
    QByteArray m_receivedHeaders;
    int m_port;
    bool m_ssl;
};

#endif /* HTTPSERVER_P_H */

