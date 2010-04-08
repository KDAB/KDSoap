#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapAuthentication.h"
#include "wsdl_mywsdl.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

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

    // Test that we can use asyncCall without using a watcher, just waiting and checking isFinished.
    void testAsyncCall()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapPendingCall call = client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!call.isFinished());
        QTest::qWait(1000);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
        QVERIFY(call.isFinished());
        QCOMPARE(call.returnMessage().arguments().value(QLatin1String("employeeCountry")).toString(), QString::fromLatin1("France"));
    }

    // Test for basic auth, with async call
    void testAsyncCallWithAuth()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::BasicAuth);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("testpass"));
        client.setAuthentication(auth);
        KDSoapPendingCall call = client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!call.isFinished());
        waitForCallFinished(call);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
        QVERIFY(call.isFinished());
        QCOMPARE(call.returnMessage().arguments().value(QLatin1String("employeeCountry")).toString(), QString::fromLatin1("France"));
    }

    // Test for refused auth, with async call
    void testAsyncCallRefusedAuth()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::BasicAuth);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("invalid"));
        client.setAuthentication(auth);
        KDSoapPendingCall call = client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!call.isFinished());
        waitForCallFinished(call);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
        QVERIFY(call.isFinished());
        QVERIFY(call.returnMessage().isFault());
    }

    // Test for refused auth, with sync call (i.e. in thread)
    void testCallRefusedAuth()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::BasicAuth);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("invalid"));
        client.setAuthentication(auth);
        KDSoapMessage reply = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(reply.isFault());
    }

    // Using direct call(), check the xml we send, the response parsing.
    // Then test callNoReply, then various ways to use asyncCall.
    void testCallNoReply()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::Public);

        // First, make the proper call
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("unused"));
        client.setAuthentication(auth); // unused...
        QByteArray expectedRequestXml = expectedCountryRequest();

        {
            KDSoapMessage ret = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
            // Check what we sent
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QVERIFY(!ret.isFault());
            QCOMPARE(ret.arguments().value(QLatin1String("employeeCountry")).toString(), QString::fromLatin1("France"));
        }

        // Now make the call again, but async, and don't wait for response.
        server.resetReceivedBuffers();
        //qDebug() << "== now calling callNoReply ==";
        client.callNoReply(QLatin1String("getEmployeeCountry"), countryMessage());
        QTest::qWait(200);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        // What happens if we use asyncCall and discard the result?
        // The KDSoapPendingCall goes out of scope, and the network request is aborted.
        //
        // The whole reason KDSoapPendingCall is a value, is so that people don't forget
        // to delete it. But of course if they even forget to store it, nothing happens.
        server.resetReceivedBuffers();
        {
            client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
            QTest::qWait(200);
        }
        QVERIFY(server.receivedData().isEmpty());
    }

    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testMyWsdl()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "<soap:Body>"
                                  "<kdab:addEmployeeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl.wsdl\"><kdab:bStrReturn>Foo</kdab:bStrReturn></kdab:addEmployeeResponse>"
                                  " </soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public /*TODO ssl test*/);

        // For testing the http server with telnet or wget:
        //httpGet(server.endPoint());
        //QEventLoop testLoop;
        //testLoop.exec();

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
    }

    // Test parsing of complex replies, like with SugarCRM
    void testParseComplexReply()
    {
        HttpServerThread server(complexTypeResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        const KDSoapMessage reply = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!reply.isFault());
        QCOMPARE(reply.arguments().count(), 1);
        const KDSoapValueList lst = qVariantValue<KDSoapValueList>(reply.arguments().first().value());
        QCOMPARE(lst.count(), 2);
        const KDSoapValue id = lst.first();
        QCOMPARE(id.name(), QString::fromLatin1("id"));
        QCOMPARE(id.value().toString(), QString::fromLatin1("12345"));
        const KDSoapValue error = lst.at(1);
        QCOMPARE(error.name(), QString::fromLatin1("error"));
        const KDSoapValueList errorList = qVariantValue<KDSoapValueList>(error.value());
        QCOMPARE(errorList.count(), 3);
        const KDSoapValue number = errorList.at(0);
        QCOMPARE(number.name(), QString::fromLatin1("number"));
        QCOMPARE(number.value().toString(), QString::fromLatin1("0"));
        const KDSoapValue name = errorList.at(1);
        QCOMPARE(name.name(), QString::fromLatin1("name"));
        QCOMPARE(name.value().toString(), QString::fromLatin1("No Error"));
        const KDSoapValue description = errorList.at(2);
        QCOMPARE(description.name(), QString::fromLatin1("description"));
        QCOMPARE(description.value().toString(), QString::fromLatin1("No Error"));
        //qDebug() << lst;
    }

private:
    static QByteArray countryResponse() {
        return QByteArray(xmlEnvBegin) + "<soap:Body>"
                "<kdab:getEmployeeCountryResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl.wsdl\"><kdab:employeeCountry>France</kdab:employeeCountry></kdab:getEmployeeCountryResponse>"
                " </soap:Body>" + xmlEnvEnd;
    }
    static QByteArray expectedCountryRequest() {
        return QByteArray(xmlEnvBegin) +
                "<soap:Body>"
                "<n1:getEmployeeCountry xmlns:n1=\"http://www.kdab.com/xml/MyWsdl.wsdl\">"
                "<n1:employeeName xsi:type=\"xsd:string\">David Faure</n1:employeeName>"
                "</n1:getEmployeeCountry>"
                "</soap:Body>" + xmlEnvEnd;
    }
    static QString countryMessageNamespace() {
        return QString::fromLatin1("http://www.kdab.com/xml/MyWsdl.wsdl");
    }
    static KDSoapMessage countryMessage() {
        KDSoapMessage message;
        message.addArgument(QLatin1String("employeeName"), QLatin1String("David Faure"));
        return message;
    }
    void waitForCallFinished(KDSoapPendingCall& pendingCall)
    {
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        QEventLoop loop;
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                &loop, SLOT(quit()));
        loop.exec();

    }

    static QByteArray complexTypeResponse() {
        return QByteArray(xmlEnvBegin) + "<soap:Body xmlns:tns=\"http://www.sugarcrm.com/sugarcrm\">"
                "<ns1:loginResponse xmlns:ns1=\"http://www.sugarcrm.com/sugarcrm\">"
                "  <return xsi:type=\"tns:set_entry_result\">"
                "    <id xsi:type=\"xsd:string\">12345</id>"
                "    <error xsi:type=\"tns:error_value\">"
                "       <number xsi:type=\"xsd:string\">0</number>"
                "       <name xsi:type=\"xsd:string\">No Error</name>"
                "       <description xsi:type=\"xsd:string\">No Error</description>"
                "    </error>"
                "  </return>"
                "</ns1:loginResponse>"
                "</soap:Body>" + xmlEnvEnd;
    }
};

QTEST_MAIN(BuiltinHttpTest)

#include "builtinhttp.moc"
