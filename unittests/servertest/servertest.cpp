#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapServer.h"
#include "KDSoapThreadPool.h"
#include "KDSoapServerObjectInterface.h"
#include "httpserver_p.h" // KDSoapUnitTestHelpers
#include <QtTest/QtTest>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#ifndef QT_NO_OPENSSL
#include <QSslConfiguration>
#endif

class CountryServerObject;
typedef QMap<QThread*, CountryServerObject*> ServerObjectsMap;
ServerObjectsMap s_serverObjects;
QMutex s_serverObjectsMutex;

class PublicThread : public QThread
{
public:
    using QThread::msleep;
};

class CountryServerObject : public QObject, public KDSoapServerObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(KDSoapServerObjectInterface)
public:
    CountryServerObject() : QObject(), KDSoapServerObjectInterface() {
        //qDebug() << "Server object created in thread" << QThread::currentThread();
        QMutexLocker locker(&s_serverObjectsMutex);
        s_serverObjects.insert(QThread::currentThread(), this);
    }
    ~CountryServerObject() {
        QMutexLocker locker(&s_serverObjectsMutex);
        Q_ASSERT(s_serverObjects.value(QThread::currentThread()) == this);
        s_serverObjects.remove(QThread::currentThread());
    }

    virtual void processRequest(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray& soapAction);
public: // SOAP-accessible methods
    QString getEmployeeCountry(const QString& employeeName) {
        // Should be called in same thread as constructor
        s_serverObjectsMutex.lock();
        Q_ASSERT(s_serverObjects.value(QThread::currentThread()) == this);
        s_serverObjectsMutex.unlock();
        if (employeeName.isEmpty()) {
            setFault(QLatin1String("Client.Data"), QLatin1String("Empty employee name"),
                     QLatin1String("CountryServerObject"), tr("Employee name must not be empty"));
            return QString();
        }
        //qDebug() << "getEmployeeCountry(" << employeeName << ") called";
        //if (employeeName == QLatin1String("Slow"))
        //    PublicThread::msleep(100);
        return QString::fromLatin1("France");
    }

    double getStuff(int foo, float bar, const QDateTime& dateTime) {
        //qDebug() << "getStuff called:" << foo << bar << dateTime.toTime_t();
        //qDebug() << "Request headers:" << requestHeaders();
        if (soapAction() != "MySoapAction") {
            qDebug() << "ERROR: SoapAction was" << soapAction();
            return 0; // error
        }
        const QString header1 = requestHeaders().header(QString::fromLatin1("header1")).value().toString();
        if (header1 == QLatin1String("headerValue")) {
            KDSoapHeaders headers;
            KDSoapMessage header2;
            header2.addArgument(QString::fromLatin1("header2"), QString::fromLatin1("responseHeader"));
            headers.append(header2);
            setResponseHeaders(headers);
        }
        return double(foo) + bar + double(dateTime.toTime_t()) + double(dateTime.time().msec() / 1000.0);
    }
    QByteArray hexBinaryTest(const QByteArray& input1, const QByteArray& input2) const {
        if (soapAction() != "ActionHex") {
            qDebug() << "ERROR: SoapAction was" << soapAction();
            return ""; // error
        }
        return input1 + input2;
    }
};

class CountryServer : public KDSoapServer
{
    Q_OBJECT
public:
    CountryServer() : KDSoapServer() {}
    virtual QObject* createServerObject() { return new CountryServerObject; }

Q_SIGNALS:
    void releaseSemaphore();

public Q_SLOTS:
    void quit() { thread()->quit(); }
    void suspend() { KDSoapServer::suspend(); emit releaseSemaphore(); }
    void resume() { KDSoapServer::resume(); emit releaseSemaphore(); }
};

// We need to do the listening and socket handling in a separate thread,
// so that the main thread can use synchronous calls. Note that this is
// really specific to unit tests and doesn't need to be done in a real
// KDSoap-based server.
class CountryServerThread : public QThread
{
    Q_OBJECT
public:
    CountryServerThread(KDSoapThreadPool* pool = 0)
        : m_threadPool(pool)
    {}
    ~CountryServerThread() {
        // helgrind says don't call quit() here, it races with exec()
        if (m_pServer)
            QMetaObject::invokeMethod(m_pServer, "quit");
        wait();
    }
    CountryServer* startThread() {
        start();
        m_semaphore.acquire(); // wait for init to be done
        return m_pServer;
    }
    void suspend() {
        QMetaObject::invokeMethod(m_pServer, "suspend");
        m_semaphore.acquire();
    }
    void resume() {
        QMetaObject::invokeMethod(m_pServer, "resume");
        m_semaphore.acquire();
    }

protected:
    void run() {
        CountryServer server;
        if (m_threadPool)
            server.setThreadPool(m_threadPool);
        if (server.listen())
            m_pServer = &server;
        connect(&server, SIGNAL(releaseSemaphore()), this, SLOT(slotReleaseSemaphore()), Qt::DirectConnection);
        m_semaphore.release();
        exec();
        m_pServer = 0;
    }
private Q_SLOTS:
    void slotReleaseSemaphore() {
        m_semaphore.release();
    }

private:
    KDSoapThreadPool* m_threadPool;
    QSemaphore m_semaphore;
    CountryServer* m_pServer;
};

class ServerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
#ifndef QT_NO_OPENSSL
        QVERIFY(KDSoapUnitTestHelpers::setSslConfiguration());
        QSslConfiguration defaultConfig = QSslConfiguration::defaultConfiguration();
        QFile certFile(QString::fromLatin1("../certs/test-127.0.0.1-cert.pem"));
        if (certFile.open(QIODevice::ReadOnly))
            defaultConfig.setLocalCertificate(QSslCertificate(certFile.readAll()));
        QFile keyFile(QString::fromLatin1("../certs/test-127.0.0.1-key.pem"));
        if (keyFile.open(QIODevice::ReadOnly))
            defaultConfig.setPrivateKey(QSslKey(keyFile.readAll(), QSsl::Rsa));
        QSslConfiguration::setDefaultConfiguration(defaultConfig);
#endif
    }

    void testCall()
    {
        {
            CountryServerThread serverThread;
            CountryServer* server = serverThread.startThread();

            //qDebug() << "server ready, proceeding" << server->endPoint();
            KDSoapClientInterface* client = new KDSoapClientInterface(server->endPoint(), countryMessageNamespace());
            const KDSoapMessage response = client->call(QLatin1String("getEmployeeCountry"), countryMessage());
            QCOMPARE(response.childValues().first().value().toString(), QString::fromLatin1("France"));

            QCOMPARE(s_serverObjects.count(), 1);
            QVERIFY(s_serverObjects.value(&serverThread)); // request handled by server thread itself (no thread pool)
            QCOMPARE(server->numConnectedSockets(), 1);
            delete client;
            QTest::qWait(100);
            QCOMPARE(server->numConnectedSockets(), 0);
        }
        QCOMPARE(s_serverObjects.count(), 0);
    }

    void testParamTypes()
    {
        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();

        KDSoapClientInterface client(server->endPoint(), countryMessageNamespace());
        KDSoapMessage message;
        message.addArgument(QLatin1String("foo"), 4);
        message.addArgument(QLatin1String("bar"), float(3.2));
        QDateTime dt = QDateTime::fromTime_t(123456);
        dt.setTime(dt.time().addMSecs(789));
        message.addArgument(QLatin1String("dateTime"), dt);

        // Add a header
        KDSoapMessage header1;
        header1.addArgument(QString::fromLatin1("header1"), QString::fromLatin1("headerValue"));
        KDSoapHeaders headers;
        headers << header1;

        const KDSoapMessage response = client.call(QLatin1String("getStuff"), message, QString::fromLatin1("MySoapAction"), headers);
        if (response.isFault()) {
            qDebug() << response.faultAsString();
            QVERIFY(!response.isFault());
        }
        QCOMPARE(response.value().toDouble(), double(4+3.2+123456.789));
        const KDSoapHeaders responseHeaders = client.lastResponseHeaders();
        //qDebug() << responseHeaders;
        QCOMPARE(responseHeaders.header(QString::fromLatin1("header2")).value().toString(), QString::fromLatin1("responseHeader"));
    }

    void testHexBinary()
    {
        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();

        //qDebug() << "server ready, proceeding" << server->endPoint();
        KDSoapClientInterface client(server->endPoint(), countryMessageNamespace());
        client.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        KDSoapMessage message;
        message.addArgument(QLatin1String("a"), QByteArray("KD"), KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("base64Binary"));
        message.addArgument(QLatin1String("b"), QByteArray("Soap"), KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("hexBinary"));
        const KDSoapMessage response = client.call(QLatin1String("hexBinaryTest"), message, QString::fromLatin1("ActionHex"));
        QCOMPARE(QString::fromLatin1(QByteArray::fromBase64(response.value().toByteArray()).constData()), QString::fromLatin1("KDSoap"));
    }

    void testMethodNotFound()
    {
        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();

        KDSoapClientInterface client(server->endPoint(), countryMessageNamespace());
        KDSoapMessage message;
        //QTest::ignoreMessage(QtDebugMsg, "Slot not found: \"doesNotExist\" [soapAction = \"http://www.kdab.com/xml/MyWsdl/doesNotExist\" ]");
        const KDSoapMessage response = client.call(QLatin1String("doesNotExist"), message);
        QVERIFY(response.isFault());
        QCOMPARE(response.arguments().child(QLatin1String("faultcode")).value().toString(), QString::fromLatin1("Server.MethodNotFound"));
        QCOMPARE(response.arguments().child(QLatin1String("faultstring")).value().toString(), QString::fromLatin1("doesNotExist not found"));
    }

    void testMissingParams()
    {
        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();

        KDSoapClientInterface client(server->endPoint(), countryMessageNamespace());
        KDSoapMessage message;
        message.addArgument(QLatin1String("foo"), 4);
        const KDSoapMessage response = client.call(QLatin1String("getStuff"), message);
        QVERIFY(response.isFault());
        QCOMPARE(response.faultAsString(), QString::fromLatin1("Fault code Server.RequiredArgumentMissing: bar,dateTime"));
    }

    void testThreadPoolBasic()
    {
        {
            KDSoapThreadPool threadPool;
            CountryServerThread serverThread(&threadPool);
            CountryServer* server = serverThread.startThread();

            KDSoapClientInterface* client = new KDSoapClientInterface(server->endPoint(), countryMessageNamespace());
            const KDSoapMessage response = client->call(QLatin1String("getEmployeeCountry"), countryMessage());
            QCOMPARE(response.childValues().first().value().toString(), QString::fromLatin1("France"));
            QCOMPARE(s_serverObjects.count(), 1);
            QThread* thread = s_serverObjects.begin().key();
            QVERIFY(thread != qApp->thread());
            QVERIFY(thread != &serverThread);
            QCOMPARE(server->numConnectedSockets(), 1);
            delete client;
            QTest::qWait(100); // race: we're waiting for the thread to be told about the socket disconnecting
            QCOMPARE(server->numConnectedSockets(), 0);
        }
        QCOMPARE(s_serverObjects.count(), 0);
    }

    void testMultipleThreads_data()
    {
        QTest::addColumn<int>("maxThreads");
        QTest::addColumn<int>("numRequests");
        QTest::addColumn<int>("numClients");
        QTest::addColumn<int>("expectedServerObjects");

        // QNetworkAccessManager only does 6 concurrent http requests
        // (QHttpNetworkConnectionPrivate::defaultChannelCount = 6)
        // so with numRequests > 6, don't expect more than 6 threads being used; for this
        // we would need more than one QNAM, i.e. more than one KDSoapClientInterface.

        QTest::newRow("5 parallel requests") << 5 << 5 << 1 << 5;
        QTest::newRow("5 requests in 3 threads") << 3 << 5 << 1 << 3;
        QTest::newRow("3 requests in 3 threads, from 2 clients") << 3 << 3 << 2 << 3; // this one reuses the idle threads
    }

    void testMultipleThreads()
    {
        QFETCH(int, maxThreads);
        QFETCH(int, numRequests);
        QFETCH(int, numClients);
        QFETCH(int, expectedServerObjects);
        {
            KDSoapThreadPool threadPool;
            threadPool.setMaxThreadCount(maxThreads);
            CountryServerThread serverThread(&threadPool);
            CountryServer* server = serverThread.startThread();
            for (int i = 0; i < numClients; ++i) {
                if (i > 0)
                    QTest::qWait(100); // handle disconnection from previous client
                //qDebug() << "Creating new client";
                KDSoapClientInterface client(server->endPoint(), countryMessageNamespace());
                m_returnMessages.clear();
                m_expectedMessages = numRequests;

                makeAsyncCalls(client, numRequests);
                QCOMPARE(server->numConnectedSockets(), 0); // too early, none of them connected yet
                m_eventLoop.exec();
                QCOMPARE(server->numConnectedSockets(), numRequests); // works, but racy, by design.

                QCOMPARE(m_returnMessages.count(), m_expectedMessages);
                Q_FOREACH(const KDSoapMessage& response, m_returnMessages) {
                    QCOMPARE(response.childValues().first().value().toString(), QString::fromLatin1("France"));
                }
                QCOMPARE(s_serverObjects.count(), expectedServerObjects);
                QMapIterator<QThread*, CountryServerObject*> it(s_serverObjects);
                while (it.hasNext()) {
                    QThread* thread = it.next().key();
                    QVERIFY(thread != qApp->thread());
                    QVERIFY(thread != &serverThread);
                }
            }
        }
        QCOMPARE(s_serverObjects.count(), 0);
    }

    void testMultipleThreadsMultipleClients_data()
    {
        QTest::addColumn<int>("maxThreads");
        QTest::addColumn<int>("numClients"); // number of "client interface" instances
        QTest::addColumn<int>("numRequests"); // number of requests per client interface (maximum 6)

        QTest::newRow("300 requests") << 5 << 50 << 6;
#ifndef Q_OS_MAC
#ifndef Q_OS_WIN // builbot gets "Fault code 99: Unknown error" after 358 connected sockets
        QTest::newRow("500 requests") << 5 << 125 << 4;
        QTest::newRow("600 requests, requires >1K fd") << 5 << 100 << 6;
        //QTest::newRow("1800 requests") << 5 << 300 << 6;
        QTest::newRow("3000 requests, requires >4K fd") << 5 << 500 << 6;
        QTest::newRow("10000 requests") << 5 << 1700 << 6;
#endif
#endif

        // Performance results (on a dual-core linux laptop)
        // time sudo ./server testMultipleThreadsMultipleClients:'10000 requests' ("total" number of seconds)
        // In release mode with Qt 4.6 in debug mode but a one-line change in QXmlStreamReaderPrivate::getChar: 32s total
        // In release mode with Qt 4.7 in release mode: 20s total
    }

    void testMultipleThreadsMultipleClients()
    {
        QFETCH(int, maxThreads);
        QFETCH(int, numClients);
        QFETCH(int, numRequests);
        const int expectedConnectedSockets = numClients * numRequests;
        // the *2 is because in this unittest, we have both the client and the server socket, in the same process
        if (!KDSoapServer::setExpectedSocketCount(expectedConnectedSockets * 2)) {
            if (expectedConnectedSockets > 500)
                QSKIP("needs root", SkipSingle);
            else
                QVERIFY(false); // should not happen
        }

        // Test making many more concurrent connections, using multiple QNAMs to circumvent the 6 connections limit.
        KDSoapThreadPool threadPool;
        threadPool.setMaxThreadCount(maxThreads);
        CountryServerThread serverThread(&threadPool);
        CountryServer* server = serverThread.startThread();
        QVector<KDSoapClientInterface *> clients;
        clients.resize(numClients);
        m_returnMessages.clear();
        m_expectedMessages = numRequests * numClients;
        for (int i = 0; i < numClients; ++i) {
            KDSoapClientInterface* client = new KDSoapClientInterface(server->endPoint(), countryMessageNamespace());
            clients[i] = client;

            makeAsyncCalls(*client, numRequests);
        }
        m_server = server;
        QTimer timer;
        connect(&timer, SIGNAL(timeout()), this, SLOT(slotStats()));
        timer.start(1000);

        // FOR DEBUG
        //qDebug() << server->endPoint();
        //qApp->exec();

        m_eventLoop.exec();
        slotStats();
        if (server->numConnectedSockets() < expectedConnectedSockets) {
            Q_FOREACH(const KDSoapMessage& response, m_returnMessages) {
                if (response.isFault()) {
                    qDebug() << response.faultAsString();
                    break;
                }
            }
        }
        QCOMPARE(server->numConnectedSockets(), expectedConnectedSockets);

        QCOMPARE(m_returnMessages.count(), m_expectedMessages);
        Q_FOREACH(const KDSoapMessage& response, m_returnMessages) {
            QCOMPARE(response.childValues().first().value().toString(), QString::fromLatin1("France"));
        }
        //QCOMPARE(s_serverObjects.count(), expectedServerObjects);
        qDeleteAll(clients);
    }

    void testSuspend()
    {
        KDSoapThreadPool threadPool;
        threadPool.setMaxThreadCount(6);
        CountryServerThread serverThread(&threadPool);
        CountryServer* server = serverThread.startThread();
        const QString endPoint = server->endPoint();
        KDSoapClientInterface client(endPoint, countryMessageNamespace());
        m_returnMessages.clear();
        m_expectedMessages = 2;
        makeAsyncCalls(client, m_expectedMessages);
        m_eventLoop.exec();
        QCOMPARE(server->numConnectedSockets(), m_expectedMessages);
        const quint16 oldPort = server->serverPort();
        QCOMPARE(m_returnMessages.count(), 2);

        // suspend
        serverThread.suspend();
        m_returnMessages.clear();
        m_expectedMessages = 3;
        QCOMPARE(m_returnMessages.count(), 0);

        // -> a new client can't connect at all:
        //qDebug() << "make call from new client";
        QCOMPARE(server->endPoint(), QString()); // can't use that, it's not even listening anymore
        KDSoapClientInterface client2(endPoint, countryMessageNamespace());
        makeAsyncCalls(client2, 3);
        m_eventLoop.exec();
        QCOMPARE(m_returnMessages.count(), 3);
        QCOMPARE(m_returnMessages.first().isFault(), true);
        QCOMPARE(m_returnMessages.first().faultAsString(), QString::fromLatin1("Fault code 1: Connection refused"));
        m_returnMessages.clear();

        // -> and an existing connected client shouldn't be allowed to make new calls -- TODO: force disconnect
        //qDebug() << "make call from connected client";
        m_expectedMessages = 1;
        makeAsyncCalls(client, 1);
        m_eventLoop.exec();
        QCOMPARE(m_returnMessages.count(), 1);
        QCOMPARE(m_returnMessages.first().isFault(), true);
        QCOMPARE(m_returnMessages.first().faultAsString(), QString::fromLatin1("Fault code 1: Connection refused"));
        m_returnMessages.clear();

        // resume
        m_expectedMessages = 1;
        serverThread.resume();
        QCOMPARE(server->serverPort(), oldPort);
        makeAsyncCalls(client, 1);
        m_eventLoop.exec();
        QCOMPARE(m_returnMessages.count(), 1);

        // Test calling resume again, should warn
        QTest::ignoreMessage(QtWarningMsg, "KDSoapServer: resume() called without calling suspend() first");
        serverThread.resume();
    }

    void testSuspendUnderLoad()
    {
        const int numRequests = 5;
        const int numClients = 100;
        const int maxThreads = 5;

        KDSoapThreadPool threadPool;
        threadPool.setMaxThreadCount(maxThreads);
        CountryServerThread serverThread(&threadPool);
        CountryServer* server = serverThread.startThread();
        QVector<KDSoapClientInterface *> clients;
        clients.resize(numClients);
        m_returnMessages.clear();
        m_expectedMessages = numRequests * numClients;
        for (int i = 0; i < numClients; ++i) {
            KDSoapClientInterface* client = new KDSoapClientInterface(server->endPoint(), countryMessageNamespace());
            clients[i] = client;
            makeAsyncCalls(*client, numRequests);
        }
        m_server = server;

        // Testing suspend/resume under load
        for (int n = 0; n < 4; ++n) {
            QTest::qWait(100);
            qDebug() << "suspend (" << n << ")";
            serverThread.suspend();
            QTest::qWait(100);
            qDebug() << "resume (" << n << ")";
            serverThread.resume();
        }

        if (m_returnMessages.count() < m_expectedMessages)
            m_eventLoop.exec();
        // Don't look at m_returnMessages or numConnectedSockets here,
        // some of them got an error, trying to connect while server was suspended.

        qDeleteAll(clients);
    }

    void testServerFault() // fault returned by server
    {
        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();
        makeFaultyCall(server->endPoint());
    }

#ifndef QT_NO_OPENSSL
    void testSsl()
    {
        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();
        server->setFeatures(KDSoapServer::Ssl);
        QVERIFY(server->endPoint().startsWith(QLatin1String("https")));
        makeSimpleCall(server->endPoint());
    }
#endif

    void testLogging()
    {
        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();

        const QString fileName = QString::fromLatin1("output.log");
        QFile::remove(fileName);
        server->setLogFileName(fileName);
        QCOMPARE(server->logFileName(), fileName);
        server->setLogLevel(KDSoapServer::LogEveryCall);

        makeSimpleCall(server->endPoint());
        makeFaultyCall(server->endPoint());
        server->flushLogFile();

        QList<QByteArray> expected;
        expected << "CALL getEmployeeCountry";
        expected << "FAULT getEmployeeCountry -- Fault code Client.Data: Empty employee name (CountryServerObject)";
        compareLines(expected, fileName);

        server->setLogLevel(KDSoapServer::LogNothing);
        makeSimpleCall(server->endPoint());
        makeFaultyCall(server->endPoint());
        server->flushLogFile();
        compareLines(expected, fileName);

        server->setLogLevel(KDSoapServer::LogFaults);
        makeSimpleCall(server->endPoint());
        makeFaultyCall(server->endPoint());
        expected << "FAULT getEmployeeCountry -- Fault code Client.Data: Empty employee name (CountryServerObject)";
        server->flushLogFile();
        compareLines(expected, fileName);

        // Now make too many connections
        server->setMaxConnections(2);
        const int numClients = 4;
        QVector<KDSoapClientInterface *> clients;
        m_expectedMessages = 2;
        m_returnMessages.clear();
        clients.resize(numClients);
        for (int i = 0; i < numClients; ++i) {
            KDSoapClientInterface* client = new KDSoapClientInterface(server->endPoint(), countryMessageNamespace());
            clients[i] = client;
            makeAsyncCalls(*client, 1);
        }
        m_eventLoop.exec();
        QTest::qWait(1000);
        QCOMPARE(m_returnMessages.count(), 2);
        expected << "ERROR Too many connections (2), incoming connection rejected";
        expected << "ERROR Too many connections (2), incoming connection rejected";
        server->flushLogFile();
        compareLines(expected, fileName);

        QFile::remove(fileName);
    }

    void testWsdlFile()
    {
        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();

        const QString fileName = QString::fromLatin1("foo.wsdl");
        QFile file(fileName);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("Hello world");
        file.flush();
        const QString pathInUrl = QString::fromLatin1("/path/to/file.wsdl");
        server->setWsdlFile(fileName, pathInUrl);

        QString url = server->endPoint();
        url.chop(1) /*trailing slash*/;
        url += pathInUrl;
        QNetworkAccessManager manager;
        QNetworkRequest request(url);
        QNetworkReply* reply = manager.get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        QCOMPARE((int)reply->error(), (int)QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), QByteArray("Hello world"));
    }

    void testSetPath_data()
    {
        QTest::addColumn<QString>("serverPath");
        QTest::addColumn<QString>("requestPath");
        QTest::addColumn<bool>("expectedSuccess");

        QTest::newRow("success on /foo") << "/foo" << "/foo" << true;
        QTest::newRow("mismatching paths") << "/foo" << "/bar" << false;
    }

    void testSetPath()
    {
        QFETCH(QString, serverPath);
        QFETCH(QString, requestPath);
        QFETCH(bool, expectedSuccess);

        CountryServerThread serverThread;
        CountryServer* server = serverThread.startThread();
        server->setPath(serverPath);
        QVERIFY(server->endPoint().endsWith(serverPath));
        const QString url = server->endPoint().remove(serverPath).append(requestPath);
        KDSoapClientInterface client(url, countryMessageNamespace());
        if (serverPath != requestPath) {
            QTest::ignoreMessage(QtWarningMsg, "Invalid path '/bar'");
        }

        const KDSoapMessage response = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
        QCOMPARE(response.isFault(), !expectedSuccess);
        if (!expectedSuccess) {
            QCOMPARE(response.arguments().child(QLatin1String("faultcode")).value().toString(), QString::fromLatin1("Client.Data"));
            QCOMPARE(response.arguments().child(QLatin1String("faultstring")).value().toString(), QString::fromLatin1("Invalid path '%1'").arg(requestPath));
        }
    }

public Q_SLOTS:
    void slotFinished(KDSoapPendingCallWatcher* watcher)
    {
        m_returnMessages.append(watcher->returnMessage());
        if (m_returnMessages.count() == m_expectedMessages)
            m_eventLoop.quit();
    }

    void slotStats()
    {
        qDebug() << m_server->numConnectedSockets() << "sockets connected. Messages received" << m_returnMessages.count();
    }

private:
    QEventLoop m_eventLoop;
    int m_expectedMessages;
    QList<KDSoapMessage> m_returnMessages;

    KDSoapServer* m_server;

private:
    void makeSimpleCall(const QString& endPoint)
    {
        KDSoapClientInterface client(endPoint, countryMessageNamespace());
        const KDSoapMessage response = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!response.isFault());
        QCOMPARE(response.childValues().first().value().toString(), QString::fromLatin1("France"));
    }

    void makeFaultyCall(const QString& endPoint)
    {
        KDSoapClientInterface client(endPoint, countryMessageNamespace());
        KDSoapMessage message;
        message.addArgument(QLatin1String("employeeName"), QString());
        const KDSoapMessage response = client.call(QLatin1String("getEmployeeCountry"), message);
        QVERIFY(response.isFault());
        QCOMPARE(response.arguments().child(QLatin1String("faultcode")).value().toString(), QString::fromLatin1("Client.Data"));
    }

    QList<KDSoapPendingCallWatcher*> makeAsyncCalls(KDSoapClientInterface& client, int numRequests)
    {
        QList<KDSoapPendingCallWatcher*> watchers;
        for (int i = 0; i < numRequests; ++i) {
            KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
            KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
            connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                    this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
            watchers.append(watcher);
        }
        return watchers;
    }

    static QString countryMessageNamespace() {
        return QString::fromLatin1("http://www.kdab.com/xml/MyWsdl/");
    }
    static KDSoapMessage countryMessage() {
        KDSoapMessage message;
        message.addArgument(QLatin1String("employeeName"), QString::fromUtf8("David Ä Faure"));
        return message;
    }

    static QList<QByteArray> readLines(const QString& fileName)
    {
        Q_ASSERT(!fileName.isEmpty());
        Q_ASSERT(QFile::exists(fileName));
        QFile file(fileName);
        const bool opened = file.open(QIODevice::ReadOnly);
        Q_ASSERT(opened);
        Q_UNUSED(opened);
        QList<QByteArray> lines;
        QByteArray line;
        do {
            line = file.readLine();
            if (!line.isEmpty())
                lines.append(line);
        } while(!line.isEmpty());
        return lines;
    }

    void compareLines(const QList<QByteArray>& expectedLines, const QString& fileName)
    {
        QList<QByteArray> lines = readLines(fileName);
        //qDebug() << lines;
        QCOMPARE(lines.count(), expectedLines.count());
        for (int i = 0; i < lines.count(); ++i) {
            QByteArray line = lines[i];
            QVERIFY(line.endsWith('\n'));
            line.chop(1);
            if (!line.endsWith(expectedLines[i])) {
                qDebug() << "line" << i << ":\n" << line << "\nexpected\n" << expectedLines[i];
                QVERIFY(line.endsWith(expectedLines[i]));
            }
        }
    }

};

QTEST_MAIN(ServerTest)

// TODO: generate this method (needs a .wsdl file)
void CountryServerObject::processRequest(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray& soapAction)
{
    setResponseNamespace(QLatin1String("http://www.kdab.com/xml/MyWsdl"));
    const QByteArray method = request.name().toLatin1();
    if (method == "getEmployeeCountry") {
        if (soapAction != "http://www.kdab.com/xml/MyWsdl/getEmployeeCountry") {
            setFault(QLatin1String("Server.UnknownSoapAction"), QLatin1String("Unknown soap action"), QLatin1String(""), QLatin1String(soapAction.constData()));
            return;
        }
        const QString employeeName = request.childValues().child(QLatin1String("employeeName")).value().toString();
        const QString ret = this->getEmployeeCountry(employeeName);
        if (!hasFault()) {
            response.setValue(QLatin1String("getEmployeeCountryResponse"));
            response.addArgument(QLatin1String("employeeCountry"), ret);
        }
    } else if (method == "getStuff") {
        const KDSoapValueList& values = request.childValues();
        const KDSoapValue valueFoo = values.child(QLatin1String("foo"));
        const KDSoapValue valueBar = values.child(QLatin1String("bar"));
        const KDSoapValue valueDateTime = values.child(QLatin1String("dateTime"));
        if (valueFoo.isNull() || valueBar.isNull() || valueDateTime.isNull()) {
            response.setFault(true);
            response.addArgument(QLatin1String("faultcode"), QLatin1String("Server.RequiredArgumentMissing"));
            QStringList argNames;
            if (valueFoo.isNull()) argNames << QLatin1String("foo");
            if (valueBar.isNull()) argNames << QLatin1String("bar");
            if (valueDateTime.isNull()) argNames << QLatin1String("dateTime");
            response.addArgument(QLatin1String("faultstring"), argNames.join(QChar::fromLatin1(',')));
            return;
        }
        const int foo = valueFoo.value().toInt();
        const float bar = valueBar.value().toFloat();
        const QDateTime dateTime = valueDateTime.value().toDateTime();
        const double ret = this->getStuff(foo, bar, dateTime);
        if (!hasFault()) {
            response.setValue(ret);
        }
    } else if (method == "hexBinaryTest") {
        const KDSoapValueList& values = request.childValues();
        const QByteArray input1 = QByteArray::fromBase64(values.child(QLatin1String("a")).value().toByteArray());
        //qDebug() << "input1=" << input1;
        const QByteArray input2 = QByteArray::fromHex(values.child(QLatin1String("b")).value().toByteArray());
        //qDebug() << "input2=" << input2;
        const QByteArray hex = this->hexBinaryTest(input1, input2);
        if (!hasFault()) {
            response.setValue(QVariant(hex));
        }
    } else {
        KDSoapServerObjectInterface::processRequest(request, response, soapAction);
    }
}


#include "servertest.moc"
