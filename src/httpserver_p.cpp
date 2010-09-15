#include "httpserver_p.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QEventLoop>

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
    //reply->ignoreSslErrors();

    QEventLoop ev;
    QObject::connect(reply, SIGNAL(finished()), &ev, SLOT(quit()));
    ev.exec();

    //QObject::connect(reply, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));
    //QTestEventLoop::instance().enterLoop(11);

    delete reply;
}

void HttpServerThread::run()
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
        QList<QByteArray> contentTypes = headers.value("Content-Type").split(';');
        if (contentTypes[0] == "text/xml" && headers.value("SoapAction").isEmpty()) {
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
            QByteArray authValue = headers.value("Authorization");
            if (authValue.isEmpty())
                authValue = headers.value("authorization"); // as sent by Qt-4.5
            bool authOk = false;
            if (!authValue.isEmpty()) {
                //qDebug() << "got authValue=" << authValue; // looks like "Basic <base64 of user:pass>"
                Method method;
                QString headerVal;
                parseAuthLine(QString::fromLatin1(authValue), &method, &headerVal);
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
    if (doDebug) {
        qDebug() << "HttpServerThread terminated";
    }
}

#include "moc_httpserver_p.cpp"
