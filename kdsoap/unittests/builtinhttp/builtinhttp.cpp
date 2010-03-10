#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "wsdl_mywsdl.h"
#include <QTcpServer>
#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#endif
#include <QtTest/QtTest>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QEventLoop>
#include <QDebug>

static void httpGet(const QUrl& url)
{
    QNetworkRequest request(url);
    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.get(request);
    //reply->ignoreSslErrors();

    QObject::connect(reply, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));
    QTestEventLoop::instance().enterLoop(11);
    delete reply;
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

    inline int serverPort() const { return m_port; }
    QString endPoint() const {
        return QString::fromLatin1("%1://127.0.0.1:%2/path")
                           .arg(QString::fromLatin1(m_ssl?"https":"http"))
                           .arg(serverPort());
    }

    inline void finish() {
        httpGet(endPoint() + QLatin1String("/terminateThread"));
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
static bool xmlBufferCompare(const QByteArray& source, const QByteArray& dest)
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

static const char* xmlEnvBegin =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\""
        " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">";
static const char* xmlEnvEnd = "</soap:Envelope>";

class BuiltinHttpTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testMyWsdl()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "<soap:Body>"
                                  "<kdab:addEmployeeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl.wsdl\"><kdab:bStrReturn>Foo</kdab:bStrReturn></kdab:addEmployeeResponse>"
                                  " </soap:Body>" + xmlEnvEnd;
        HttpServerThread server(makeHttpResponse(responseData), false /*TODO ssl test*/);

        // For testing the http server with telnet or wget:
        //httpGet(server.endPoint());
        //QEventLoop testLoop;
        //testLoop.exec();

#if 1
        MyWsdl service;
        service.setEndPoint(server.endPoint());
        KDAB__EmployeeType employeeType;
        employeeType.setType(KDAB__EmployeeTypeEnum::Developer);
        employeeType.setOtherRoles(QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader);
        employeeType.setTeam(QString::fromLatin1("Minitel"));
        QString ret = service.addEmployee(employeeType,
                                          QString::fromLatin1("David Faure"),
                                          QString::fromLatin1("France"));
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QString::fromLatin1("Foo"));
        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) +
            "<soap:Body>"
            "<n1:addEmployee xmlns:n1=\"http://www.kdab.com/xml/MyWsdl.wsdl\">"
            "<n1:employeeType>"
            "<n1:team xsi:type=\"xsd:string\">Minitel</n1:team>"
            "<n1:type xsi:type=\"xsd:string\">Developer</n1:type>"
            "<n1:otherRoles xsi:type=\"xsd:string\">TeamLeader</n1:otherRoles>"
            "</n1:employeeType>"
            "<n1:employeeName xsi:type=\"xsd:string\">David Faure</n1:employeeName>"
            "<n1:employeeCountry xsi:type=\"xsd:string\">France</n1:employeeCountry>"
            "</n1:addEmployee>"
            "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        QCOMPARE(QString::fromUtf8(server.receivedData()), QString::fromUtf8(expectedRequestXml));
        QVERIFY(server.receivedHeaders().contains("SoapAction: http://www.kdab.com/AddEmployee"));

        // Test utf8
        server.resetReceivedBuffers();
        expectedRequestXml.replace("David Faure", "Hervé");
        expectedRequestXml.replace("France", "фгн7");
        ret = service.addEmployee(employeeType,
                                  QString::fromUtf8("Hervé"),
                                  QString::fromUtf8("фгн7")); // random russian letters
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QString::fromLatin1("Foo"));
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
#endif

        server.finish();
        server.wait();
    }

    // Using direct call(), check the xml we send, the response parsing.
    // Then test callNoReply, then various ways to use asyncCall.
    void testCallNoReply()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "<soap:Body>"
                                  "<kdab:getEmployeeCountryResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl.wsdl\"><kdab:employeeCountry>France</kdab:employeeCountry></kdab:getEmployeeCountryResponse>"
                                  " </soap:Body>" + xmlEnvEnd;
        HttpServerThread server(makeHttpResponse(responseData), false /*no ssl*/);

        // First, make the proper call
        const QString messageNamespace = QString::fromLatin1("http://www.kdab.com/xml/MyWsdl.wsdl");
        KDSoapClientInterface client(server.endPoint(), messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("employeeName"), QLatin1String("David Faure"));
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin) +
                                        "<soap:Body>"
                                        "<n1:getEmployeeCountry xmlns:n1=\"http://www.kdab.com/xml/MyWsdl.wsdl\">"
                                        "<n1:employeeName xsi:type=\"xsd:string\">David Faure</n1:employeeName>"
                                        "</n1:getEmployeeCountry>"
                                        "</soap:Body>" + xmlEnvEnd;
        {
            KDSoapMessage ret = client.call(QLatin1String("getEmployeeCountry"), message);
            // Check what we sent
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QVERIFY(!ret.isFault());
            QCOMPARE(ret.arguments().value(QLatin1String("employeeCountry")).toString(), QString::fromLatin1("France"));
        }

        // Now make the call again, but async, and don't wait for response.
        server.resetReceivedBuffers();
        client.callNoReply(QLatin1String("getEmployeeCountry"), message);
        QTest::qWait(200);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        // What happens if we use asyncCall and discard the result?
        // The KDSoapPendingCall goes out of scope, and the network request is aborted.
        //
        // The whole reason KDSoapPendingCall is a value, is so that people don't forget
        // to delete it. But of course if they even forget to store it, nothing happens.
        server.resetReceivedBuffers();
        {
            client.asyncCall(QLatin1String("getEmployeeCountry"), message);
            QTest::qWait(200);
        }
        QVERIFY(server.receivedData().isEmpty());

        // And if we do asyncCall without using a watcher?
        {
            KDSoapPendingCall call = client.asyncCall(QLatin1String("getEmployeeCountry"), message);
            QVERIFY(!call.isFinished());
            QTest::qWait(200);
            QVERIFY(call.isFinished());
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QCOMPARE(call.returnMessage().arguments().value(QLatin1String("employeeCountry")).toString(), QString::fromLatin1("France"));
        }

        server.finish();
        server.wait();
    }

private:
    static QByteArray makeHttpResponse(const QByteArray& responseData)
    {
        QByteArray httpResponse("HTTP/1.0 200 OK\r\nContent-Type: text/xml\r\nContent-Length: ");
        httpResponse += QByteArray::number(responseData.size());
        httpResponse += "\r\n\r\n";
        httpResponse += responseData;
        return httpResponse;
    }
};

QTEST_MAIN(BuiltinHttpTest)

#include "builtinhttp.moc"
