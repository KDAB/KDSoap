#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
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
    }
};

QTEST_MAIN(BuiltinHttpTest)

#include "builtinhttp.moc"
