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
#include <QEventLoop>
#include <QDebug>

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
        waitForNewConnection(-1);
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
        : m_dataToSend(dataToSend), m_ssl(ssl), m_finish(false)
    {
        start();
        m_ready.acquire();

    }

    inline int serverPort() const { return m_port; }
    inline void finish() { m_finish = true; }

    /* \reimp */ void run()
    {
        BlockingHttpServer server(m_ssl);
        server.listen();
        m_port = server.serverPort();
        m_ready.release();

        //qDebug() << "HttpServerThread listening on port" << m_port;

        QTcpSocket *clientSocket = server.waitForNextConnectionSocket();
        while (!m_finish) {
            // get the "request" packet
            if (!clientSocket->waitForReadyRead(2000)) {
                if (m_finish)
                    break;
                qDebug() << "HttpServerThread:" << clientSocket->error() << "waiting for \"request\" packet";
                break;
            }
            m_dataReceived = clientSocket->readAll();
            //qDebug() << "data received:" << m_dataReceived;
            // send response
            //qDebug() << "writing" << m_dataToSend;
            clientSocket->write( m_dataToSend );
            clientSocket->flush();
        }
        // all done...
        clientSocket->disconnectFromHost();
    }

private:
    QSemaphore m_ready;
    QByteArray m_dataToSend;
    QByteArray m_dataReceived;
    int m_port;
    bool m_ssl;
    bool m_finish;
};


class BuiltinHttpTest : public QObject
{
    Q_OBJECT
public:

private Q_SLOTS:
    void testMyWsdl()
    {
        QByteArray responseData =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\""
                " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\""
                " xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl.wsdl\""
                " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><soap:Body>"
                "<kdab:addEmployeeResponse><kdab:bStrReturn>Foo</kdab:bStrReturn></kdab:addEmployeeResponse>"
                " </soap:Body></soap:Envelope>";
        QByteArray httpResponse("HTTP/1.0 200 OK\r\nContent-Type: text/xml\r\nContent-Length: ");
        httpResponse += QByteArray::number(responseData.size());
        httpResponse += "\r\n\r\n";
        httpResponse += responseData;
        HttpServerThread server(httpResponse, false /*TODO ssl test*/);

        const QString endPoint = QString::fromLatin1("%1://127.0.0.1:%2/path")
                           .arg(QString::fromLatin1(false/*TODO*/?"https":"http"))
                           .arg(server.serverPort());
#if 1
        MyWsdl service;
        service.setEndPoint(endPoint);
        KDAB__EmployeeType employeeType;
        employeeType.setType(KDAB__EmployeeTypeEnum::Developer);
        employeeType.setOtherRoles(QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader);
        employeeType.setTeam(QString::fromLatin1("Minitel"));
        QString ret = service.addEmployee(employeeType, KDAB__EmployeeName(QString::fromLatin1("David")), QString::fromLatin1("France"));
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QString::fromLatin1("Foo"));
#endif

#if 0
        QUrl url(endPoint);
        QNetworkRequest request(url);
        QNetworkAccessManager manager;
        QNetworkReply* reply = manager.get(request);
        //reply->ignoreSslErrors();

        connect(reply, SIGNAL(finished()), &QTestEventLoop::instance(), SLOT(exitLoop()));
        QTestEventLoop::instance().enterLoop(11);
        qDebug() << "OK";
        delete reply;
#endif

        // For testing the http server with telnet or wget:
        //QEventLoop testLoop;
        //testLoop.exec();

        server.finish();
        server.wait();
    }
};

QTEST_MAIN(BuiltinHttpTest)

#include "builtinhttp.moc"
