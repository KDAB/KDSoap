#include "wsdl_mywsdl_document.h"
#include "wsdl_thomas-bayer.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapPendingCallWatcher.h>
#include <KDSoapNamespaceManager.h>
#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#endif

using namespace KDSoapUnitTestHelpers;

static const char* xmlEnvBegin =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<soap:Envelope"
        " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
        " xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\""
        " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"";
static const char* xmlEnvEnd = "</soap:Envelope>";

static const char* xmlBegin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
static const char* xmlNamespaces = "xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
    " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
    " xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\""
    " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\"";

class WsdlDocumentTest : public QObject
{
    Q_OBJECT

private:
    static KDAB__AddEmployee addEmployeeParameters()
    {
        KDAB__EmployeeAchievements achievements;
        QList<KDAB__EmployeeAchievement> lst;
        KDAB__EmployeeAchievement achievement1;
        achievement1.setType(QByteArray("Project"));
        achievement1.setLabel(QString::fromLatin1("Management"));
        lst.append(achievement1);
        KDAB__EmployeeAchievement achievement2;
        achievement2.setType(QByteArray("Development"));
        achievement2.setLabel(QString::fromLatin1("C++"));
        lst.append(achievement2);
        achievements.setItems(lst);
        KDAB__EmployeeType employeeType;
        employeeType.setType(KDAB__EmployeeTypeEnum::Developer);
        employeeType.setOtherRoles(QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader);
        employeeType.setTeam(QString::fromLatin1("Minitel"));

        KDAB__AddEmployee addEmployeeParams;
        addEmployeeParams.setEmployeeType(employeeType);
        addEmployeeParams.setEmployeeName(QString::fromLatin1("David Faure"));
        addEmployeeParams.setEmployeeCountry(QString::fromLatin1("France"));
        addEmployeeParams.setEmployeeAchievements(achievements);
        KDAB__EmployeeId id;
        id.setId(5);
        addEmployeeParams.setEmployeeId(id);
        return addEmployeeParams;
    }

    static QByteArray requestXmlTemplate()
    {
        return QByteArray(xmlEnvBegin) +
                " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">%1"
                "<soap:Body>"
                "<n1:addEmployee>"
                "<n1:employeeType n1:type=\"Developer\">"
                "<n1:team>Minitel</n1:team>"
                "<n1:otherRoles>TeamLeader</n1:otherRoles>"
                "</n1:employeeType>"
                "<n1:employeeName>David Faure</n1:employeeName>"
                "<n1:employeeCountry>France</n1:employeeCountry>"
                "<n1:employeeAchievements>"
                "<n1:item>"
                "<n1:type>50726f6a656374</n1:type>" // Project
                "<n1:label>Management</n1:label>"
                "</n1:item>"
                "<n1:item>"
                "<n1:type>446576656c6f706d656e74</n1:type>" // Development
                "<n1:label>C++</n1:label>"
                "</n1:item>"
                "</n1:employeeAchievements>"
                "<n1:employeeId>"
                "<n1:id>5</n1:id>"
                "</n1:employeeId>"
                "</n1:addEmployee>"
                "</soap:Body>" + xmlEnvEnd
                + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
    static QByteArray expectedHeader() {
        return QByteArray("<soap:Header>"
                          "<n1:LoginElement>"
                          "<n1:user>foo</n1:user>"
                          "<n1:pass>bar</n1:pass>"
                          "</n1:LoginElement>"
                          "<n1:SessionElement>"
                          "<n1:sessionId>id</n1:sessionId>"
                          "</n1:SessionElement>"
                          "</soap:Header>");
    }
    static QByteArray addEmployeeResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body>"
                "<kdab:addEmployeeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">466F6F</kdab:addEmployeeResponse>"
                " </soap:Body>" + xmlEnvEnd;
    }

private Q_SLOTS:
    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testMyWsdlPublic()
    {
        HttpServerThread server(addEmployeeResponse(), HttpServerThread::Public);

        // For testing the http server with telnet or wget:
        //httpGet(server.endPoint());
        //QEventLoop testLoop;
        //testLoop.exec();

        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__LoginElement login;
        login.setUser(QLatin1String("foo"));
        login.setPass(QLatin1String("bar"));
        KDAB__SessionElement session;
        session.setSessionId(QLatin1String("id"));

        service.setLoginHeader(login);
        service.setSessionHeader(session);

        KDAB__AddEmployee addEmployeeParams = addEmployeeParameters();

        QByteArray ret = service.addEmployee(addEmployeeParams);
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QByteArray("Foo"));
        // Check what we sent
        {
            QByteArray expectedRequestXml = requestXmlTemplate();
            expectedRequestXml.replace("%1", expectedHeader());
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedRequestXml.constData()));
            QVERIFY(server.receivedHeaders().contains("SoapAction: http://www.kdab.com/AddEmployee"));
        }

        // Test utf8
        addEmployeeParams.setEmployeeName(QString::fromUtf8("Hervé"));
        addEmployeeParams.setEmployeeCountry(QString::fromUtf8("фгн7")); // random russian letters
        {
            // This second call also tests that persistent headers are indeed persistent.
            server.resetReceivedBuffers();
            QByteArray expectedRequestXml = requestXmlTemplate();
            expectedRequestXml.replace("%1", expectedHeader());
            expectedRequestXml.replace("David Faure", "Hervé");
            expectedRequestXml.replace("France", "фгн7");
            ret = service.addEmployee(addEmployeeParams);
            QVERIFY(service.lastError().isEmpty());
            QCOMPARE(ret, QByteArray("Foo"));
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        }

        // Test removing headers
        {
            server.resetReceivedBuffers();
            service.clearLoginHeader();
            service.clearSessionHeader();
            ret = service.addEmployee(addEmployeeParameters());
            QByteArray expectedRequestXml = requestXmlTemplate();
            expectedRequestXml.replace("%1", "<soap:Header/>");
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        }
    }

#ifndef QT_NO_OPENSSL
    void testMyWsdlSSL()
    {
        if (!QSslSocket::supportsSsl()) {
            QSKIP("No SSL support on this machine, check that ssleay.so/ssleay32.dll is installed", SkipAll);
        }

#ifndef QT_NO_SSLSOCKET
        QVERIFY(KDSoapUnitTestHelpers::setSslConfiguration());
#endif

        HttpServerThread server(addEmployeeResponse(), HttpServerThread::Ssl);

        // For testing the http server with telnet or wget:
        //qDebug() << "endPoint=" << server.endPoint();
        //httpGet(server.endPoint());
        //QEventLoop testLoop;
        //testLoop.exec();

        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());
        QVERIFY(server.endPoint().startsWith(QLatin1String("https")));

        KDAB__LoginElement login;
        login.setUser(QLatin1String("foo"));
        login.setPass(QLatin1String("bar"));
        KDAB__SessionElement session;
        session.setSessionId(QLatin1String("id"));

        service.setLoginHeader(login);
        service.setSessionHeader(session);

        QByteArray ret = service.addEmployee(addEmployeeParameters());
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();

        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QByteArray("Foo"));
        // Check what we sent
        QByteArray expectedRequestXml = requestXmlTemplate();
        expectedRequestXml.replace("%1", expectedHeader());
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedRequestXml.constData()));
        QVERIFY(server.receivedHeaders().contains("SoapAction: http://www.kdab.com/AddEmployee"));
    }
#endif

    void testSimpleType()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__EmployeeNameParams params;
        params.setEmployeeName(KDAB__EmployeeName(QString::fromUtf8("David Ä Faure")));
        const KDAB__EmployeeCountryResponse employeeCountryResponse = service.getEmployeeCountry(params);
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(employeeCountryResponse.employeeCountry().value(), QString::fromLatin1("France"));
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
        QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedCountryRequest().constData()));
    }

    void testEmptyResponse()
    {
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());
        KDAB__EmployeeNameParams params;
        const KDAB__EmployeeCountryResponse employeeCountryResponse = service.getEmployeeCountry(params);
        QCOMPARE(service.lastError(), QString()); // no error, just an empty struct
        QCOMPARE(employeeCountryResponse.employeeCountry().value(), QString());
    }

    // Test enum deserialization
    void testEnums()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<kdab:getEmployeeTypeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\" kdab:type=\"Developer\">"
                                    "<kdab:team>Minitel</kdab:team>"
                                    "<kdab:otherRoles>TeamLeader</kdab:otherRoles>"
                                  "</kdab:getEmployeeTypeResponse>"
                                  "</soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__EmployeeNameParams params;
        params.setEmployeeName(KDAB__EmployeeName(QLatin1String("Joe")));
        const KDAB__EmployeeType employeeType = service.getEmployeeType(params);
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(employeeType.team().value().value(), QLatin1String("Minitel"));
        QCOMPARE(employeeType.otherRoles().count(), 1);
        QCOMPARE(employeeType.otherRoles().at(0).type(), KDAB__EmployeeTypeEnum::TeamLeader);
        QCOMPARE((int)employeeType.type().type(), (int)KDAB__EmployeeTypeEnum::Developer);
    }

    void testSoapVersion()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<kdab:getEmployeeTypeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\" kdab:type=\"Developer\">"
                                    "<kdab:team>Minitel</kdab:team>"
                                    "<kdab:otherRoles>TeamLeader</kdab:otherRoles>"
                                  "</kdab:getEmployeeTypeResponse>"
                                  "</soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__EmployeeNameParams params;
        params.setEmployeeName(KDAB__EmployeeName(QLatin1String("Joe")));

        service.setSoapVersion(KDSoapClientInterface::SOAP1_1);
        KDAB__EmployeeType employeeType = service.getEmployeeType(params);
        QVERIFY(service.lastError().isEmpty());

        service.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        KDAB__EmployeeType employeeType2 = service.getEmployeeType(params);
        QVERIFY(service.lastError().isEmpty());
    }

    // Was http://www.service-repository.com/service/wsdl?id=163859, but it disappeared.
    void testSequenceInResponse()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<tb:getCountriesResponse xmlns:tb=\"http://namesservice.thomas_bayer.com/\"><tb:country>Great Britain</tb:country><tb:country>Ireland</tb:country></tb:getCountriesResponse>"
                                  " </soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);

        NamesServiceService serv;
        serv.setEndPoint(server.endPoint());
        const QStringList countries = serv.getCountries().country(); // the wsdl should have named it "countries"...
        QCOMPARE(countries.count(), 2);
        QCOMPARE(countries[0], QString::fromLatin1("Great Britain"));
        QCOMPARE(countries[1], QString::fromLatin1("Ireland"));

        const QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) + ">"
            "<soap:Body>"
            "<n1:getCountries xmlns:n1=\"http://namesservice.thomas_bayer.com/\"/>"
            "</soap:Body>" + xmlEnvEnd;
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        // Same test without using generated code
        {
            const QString messageNamespace = QString::fromLatin1("http://namesservice.thomas_bayer.com/");
            KDSoapClientInterface client(server.endPoint(), messageNamespace);
            KDSoapMessage message;
            KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("getCountries"), message);
            KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
            connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                    this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
            m_eventLoop.exec();
            //qDebug() << m_returnMessage;

            QCOMPARE(m_returnMessage.arguments()[0].value().toString(), QString::fromLatin1("Great Britain"));
            QCOMPARE(m_returnMessage.arguments()[1].value().toString(), QString::fromLatin1("Ireland"));
        }
    }

    void testAnyType()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<kdab:AnyTypeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">"
                                  "<xsd:schema><xsd:test>response</xsd:test></xsd:schema>"
                                  "<kdab:return xsd:type=\"xsd:int\">42</kdab:return>"
                                  "<kdab:return xsd:type=\"xsd:string\">Forty-two</kdab:return>"
                                  "<kdab:return xsd:type=\"kdab:TeamName\">Minitel</kdab:return>"
                                  "<kdab:return xsd:type=\"kdab:EmployeeAchievement\">"
                                    "<kdab:type>Project</kdab:type>"
                                    "<kdab:label>Management</kdab:label>"
                                  "</kdab:return>"
                                  "</kdab:AnyTypeResponse>"
                                  "</soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__AnyType anyType;
        anyType.setInput(KDSoapValue(QString::fromLatin1("foo"), QString::fromLatin1("Value"), KDSoapNamespaceManager::xmlSchema1999(), QString::fromLatin1("string")));
        KDSoapValueList schemaChildValues;
        KDSoapValue testVal(QLatin1String("test"), QString::fromLatin1("input"));
        testVal.setNamespaceUri(KDSoapNamespaceManager::xmlSchema1999());
        schemaChildValues.append(testVal);
        KDSoapValue inputSchema(QLatin1String("schema"), schemaChildValues);
        inputSchema.setNamespaceUri(KDSoapNamespaceManager::xmlSchema1999());
        anyType.setSchema(inputSchema);
        const KDAB__AnyTypeResponse response = service.testAnyType(anyType);
        const QList<KDSoapValue> values = response._return();
        QCOMPARE(values.count(), 4);
        QCOMPARE(values.at(0).value().toInt(), 42);
        QCOMPARE(values.at(1).value().toString(), QString::fromLatin1("Forty-two"));
        QCOMPARE(values.at(2).value().toString(), QString::fromLatin1("Minitel"));
        const QList<KDSoapValue> achievements = values.at(3).childValues();
        QCOMPARE(achievements.count(), 2);
        QCOMPARE(achievements.at(0).value().toString(), QString::fromLatin1("Project"));
        QCOMPARE(achievements.at(1).value().toString(), QString::fromLatin1("Management"));

        const QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) + ">"
            "<soap:Body>"
            "<n1:AnyType xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
            "<xsd:schema><xsd:test>input</xsd:test></xsd:schema>"
            "<n1:foo>Value</n1:foo>"
            "</n1:AnyType>"
            "</soap:Body>" + xmlEnvEnd;
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        const KDSoapValue schema = response.schema();
        QCOMPARE(schema.name(), QString::fromLatin1("schema"));
        QCOMPARE(schema.namespaceUri(), KDSoapNamespaceManager::xmlSchema1999());
        const QByteArray expectedResponseSchemaXml =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<xsd:schema"
                " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
                " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
                " xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\""
                " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\""
                ">"
                "<xsd:test>response</xsd:test>"
                "</xsd:schema>";
        QVERIFY(xmlBufferCompare(schema.toXml(), expectedResponseSchemaXml));
    }

    // Document/literal, not wrapped -> the operation name doesn't appear
    void testByteArrays()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                "<kdab:Telegram xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">466f6f</kdab:Telegram>"
                "</soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        const KDAB__TelegramType ret = service.sendRawTelegram(KDAB__TelegramType("Hello"));
        QCOMPARE(service.lastError(), QString());
        QCOMPARE(ret.value(), QByteArray("Foo"));

        const QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) + ">"
            "<soap:Body>"
            "<n1:Telegram xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">48656c6c6f</n1:Telegram>"
            "</soap:Body>" + xmlEnvEnd;
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testServerAddEmployee();
    void testSendTelegram();

public slots:
    void slotFinished(KDSoapPendingCallWatcher* watcher)
    {
        m_returnMessage = watcher->returnMessage();
        m_eventLoop.quit();
    }

private:
    QEventLoop m_eventLoop;
    KDSoapMessage m_returnMessage;

    static QByteArray emptyResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body/>" + xmlEnvEnd;
    }
    static QByteArray countryResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body>"
                "<kdab:getEmployeeCountryResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\"><kdab:employeeCountry>France</kdab:employeeCountry></kdab:getEmployeeCountryResponse>"
                " </soap:Body>" + xmlEnvEnd;
    }
    static QByteArray expectedCountryRequest() {
        return QByteArray(xmlEnvBegin) +
                "><soap:Body>"
                "<n1:EmployeeNameParams xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
                "<n1:employeeName>"
                "David Ä Faure"
                "</n1:employeeName>"
                "</n1:EmployeeNameParams>"
                "</soap:Body>" + xmlEnvEnd
                + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
};

#include "KDSoapServerObjectInterface.h"
#include "KDSoapServer.h"

class DocServerObject : public MyWsdlDocumentServerBase
{
public:
    // TODO add a method that returns void

    QByteArray addEmployee( const KDAB__AddEmployee& parameters ) {
        qDebug() << "addEmployee called";
        return "added " + parameters.employeeName().value().value().toLatin1();
    }

    KDAB__AnyTypeResponse testAnyType( const KDAB__AnyType& parameters ) {
        KDAB__AnyTypeResponse response;
        response.setReturn(QList<KDSoapValue>() << parameters.input());
        return response;
    }

    KDAB__EmployeeCountryResponse getEmployeeCountry( const KDAB__EmployeeNameParams& employeeNameParams ) {
        KDAB__EmployeeCountryResponse resp;
        if (QString(employeeNameParams.employeeName().value()) == QLatin1String("David")) {
            resp.setEmployeeCountry(QString::fromLatin1("France"));
        } else {
            resp.setEmployeeCountry(QString::fromLatin1("Unknown country!"));
        }
        return resp;
    }

    KDAB__EmployeeType getEmployeeType( const KDAB__EmployeeNameParams& employeeNameParams ) {
        KDAB__EmployeeType type;
        if (QString(employeeNameParams.employeeName().value()) == QLatin1String("David")) {
            type.setTeam(KDAB__TeamName(QString::fromLatin1("Minitel")));
        }
        return type;
    }

    KDAB__TelegramType sendRawTelegram( const KDAB__TelegramType& telegram ) {
        return QByteArray("Got ") + telegram;
    }

    KDAB__TelegramResponse sendTelegram( const KDAB__TelegramRequest& parameters ) {
        KDAB__TelegramResponse resp;
        resp.setTelegram(QByteArray("Received ") + parameters.telegram());
        return resp;
    }

    // Normally you don't reimplement this. This is just to store req and resp for the unittest.
    void processRequest( const KDSoapMessage &request, KDSoapMessage &response, const QByteArray& soapAction ) {
        m_request = request;
        MyWsdlDocumentServerBase::processRequest(request, response, soapAction);
        m_response = response;
        //qDebug() << "processRequest: done. " << this << "Response name=" << response.name();
    }

    KDSoapMessage m_request;
    KDSoapMessage m_response;
};

class DocServer : public KDSoapServer
{
    Q_OBJECT
public:
    DocServer() : KDSoapServer(), m_lastServerObject(0) {
        setPath(QLatin1String("/xml"));
    }
    virtual QObject* createServerObject() { m_lastServerObject = new DocServerObject; return m_lastServerObject; }

    DocServerObject* lastServerObject() { return m_lastServerObject; }
Q_SIGNALS:
    void releaseSemaphore();

public Q_SLOTS:
    void quit() { thread()->quit(); }

private:
    DocServerObject* m_lastServerObject; // only for unittest purposes
};

// We need to do the listening and socket handling in a separate thread,
// so that the main thread can use synchronous calls. Note that this is
// really specific to unit tests and doesn't need to be done in a real
// KDSoap-based server.
class DocServerThread : public QThread
{
    Q_OBJECT
public:
    DocServerThread() {}
    ~DocServerThread() {
        // helgrind says don't call quit() here (Qt bug?)
        if (m_pServer)
            QMetaObject::invokeMethod(m_pServer, "quit");
        wait();
    }
    DocServer* startThread() {
        start();
        m_semaphore.acquire(); // wait for init to be done
        return m_pServer;
    }

protected:
    void run() {
        DocServer server;
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
    QSemaphore m_semaphore;
    DocServer* m_pServer;
};

void WsdlDocumentTest::testServerAddEmployee()
{
    DocServerThread serverThread;
    DocServer* server = serverThread.startThread();

    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());
    QByteArray ret = service.addEmployee(addEmployeeParameters());
    QVERIFY(server->lastServerObject());
    const QByteArray expectedResponseXml =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<addEmployeeMyResponse xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\" xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\">"
                "6164646564204461766964204661757265"
                "</addEmployeeMyResponse>\n";
    qDebug() << server->lastServerObject() << "response name" << server->lastServerObject()->m_response.name();
    // Note: that's the response as sent by the generated code.
    // But then the server socket code will call messageToXml, possibly with a method name,
    // we can't debug that here.
    QVERIFY(xmlBufferCompare(server->lastServerObject()->m_response.toXml(), expectedResponseXml));
    QCOMPARE(service.lastError(), QString());
    QCOMPARE(QString::fromLatin1(ret.constData()), QString::fromLatin1("added David Faure"));
}

void WsdlDocumentTest::testSendTelegram()
{
    DocServerThread serverThread;
    DocServer* server = serverThread.startThread();

    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());

    KDAB__TelegramRequest req;
    req.setTelegram(KDAB__TelegramType("Hello"));
    const KDAB__TelegramResponse ret = service.sendTelegram(req);
    QCOMPARE(service.lastError(), QString());
    QCOMPARE(ret.telegram().value(), QByteArray("Received Hello"));

    const QByteArray expectedRequestXml =
        QByteArray(xmlBegin) + "<n1:TelegramRequest " + xmlNamespaces + " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
           "<n1:Telegram>48656c6c6f</n1:Telegram>"
          "</n1:TelegramRequest>";
    const QString msgNS = QString::fromLatin1("http://www.kdab.com/xml/MyWsdl/");
    QVERIFY(xmlBufferCompare(server->lastServerObject()->m_request.toXml(KDSoapValue::LiteralUse, msgNS), expectedRequestXml));

    const QByteArray expectedResponseXml =
        QByteArray(xmlBegin) + "<n1:TelegramResponse " + xmlNamespaces + " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
            "<n1:Telegram>52656365697665642048656c6c6f</n1:Telegram>"
          "</n1:TelegramResponse>";
    QVERIFY(xmlBufferCompare(server->lastServerObject()->m_response.toXml(KDSoapValue::LiteralUse, msgNS), expectedResponseXml));
}

QTEST_MAIN(WsdlDocumentTest)

#include "test_wsdl_document.moc"