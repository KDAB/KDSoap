#include "wsdl_mywsdl_document.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

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

class WsdlDocumentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testMyWsdl()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<kdab:addEmployeeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\"><kdab:bStrReturn>Foo</kdab:bStrReturn></kdab:addEmployeeResponse>"
                                  " </soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public /*TODO ssl test*/);

        // For testing the http server with telnet or wget:
        //httpGet(server.endPoint());
        //QEventLoop testLoop;
        //testLoop.exec();

        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());
        KDAB__EmployeeAchievements achievements;
        QList<KDAB__EmployeeAchievement> lst;
        KDAB__EmployeeAchievement achievement1;
        achievement1.setType(QString::fromLatin1("Project"));
        achievement1.setLabel(QString::fromLatin1("Management"));
        lst.append(achievement1);
        KDAB__EmployeeAchievement achievement2;
        achievement2.setType(QString::fromLatin1("Development"));
        achievement2.setLabel(QString::fromLatin1("C++"));
        lst.append(achievement2);
        achievements.setItems(lst);
        KDAB__EmployeeType employeeType;
        employeeType.setType(KDAB__EmployeeTypeEnum::Developer);
        employeeType.setOtherRoles(QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader);
        employeeType.setTeam(QString::fromLatin1("Minitel"));

        KDAB__AddEmployeeParams addEmployeeParams;
        addEmployeeParams.setEmployeeType(employeeType);
        addEmployeeParams.setEmployeeName(QString::fromLatin1("David Faure"));
        addEmployeeParams.setEmployeeCountry(QString::fromLatin1("France"));
        addEmployeeParams.setEmployeeAchievements(achievements);

        KDAB__LoginElement login;
        login.setUser(QLatin1String("foo"));
        login.setPass(QLatin1String("bar"));
        KDAB__SessionElement session;
        session.setSessionId(QLatin1String("id"));

        service.setLoginHeader(login);
        service.setSessionHeader(session);


        QString ret = service.addEmployee(addEmployeeParams);
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QString::fromLatin1("Foo"));
        const QByteArray expectedHeader =
                "<soap:Header>"
                "<n1:LoginHeader>"
                "<n1:user xsi:type=\"xsd:string\">foo</n1:user>"
                "<n1:pass xsi:type=\"xsd:string\">bar</n1:pass>"
                "</n1:LoginHeader>"
                "<n1:SessionHeader>"
                "<n1:sessionId xsi:type=\"xsd:string\">id</n1:sessionId>"
                "</n1:SessionHeader>"
                "</soap:Header>";
        // Check what we sent
        QByteArray requestXmlTemplate =
            QByteArray(xmlEnvBegin) +
            " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">%1"
            "<soap:Body>"
            "<n1:addEmployee>"
            "<n1:employeeType>"
            "<n1:team xsi:type=\"xsd:string\">Minitel</n1:team>"
            "<n1:type xsi:type=\"xsd:string\">Developer</n1:type>"
            "<n1:otherRoles xsi:type=\"xsd:string\">TeamLeader</n1:otherRoles>"
            "</n1:employeeType>"
            "<n1:employeeName xsi:type=\"xsd:string\">David Faure</n1:employeeName>"
            "<n1:employeeCountry xsi:type=\"xsd:string\">France</n1:employeeCountry>"
            "<n1:employeeAchievements>"
            "<n1:item>"
              "<n1:type xsi:type=\"xsd:string\">Project</n1:type>"
              "<n1:label xsi:type=\"xsd:string\">Management</n1:label>"
            "</n1:item>"
            "<n1:item>"
              "<n1:type xsi:type=\"xsd:string\">Development</n1:type>"
              "<n1:label xsi:type=\"xsd:string\">C++</n1:label>"
            "</n1:item>"
            "</n1:employeeAchievements>"
            "</n1:addEmployee>"
            "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        {
            QByteArray expectedRequestXml = requestXmlTemplate;
            expectedRequestXml.replace("%1", expectedHeader);
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QCOMPARE(QString::fromUtf8(server.receivedData()), QString::fromUtf8(expectedRequestXml));
            QVERIFY(server.receivedHeaders().contains("SoapAction: http://www.kdab.com/AddEmployee"));
        }

        // Test utf8
        addEmployeeParams.setEmployeeName(QString::fromUtf8("Hervé"));
        addEmployeeParams.setEmployeeCountry(QString::fromUtf8("фгн7")); // random russian letters

        {
            // This second call also tests that persistent headers are indeed persistent.
            server.resetReceivedBuffers();
            requestXmlTemplate.replace("David Faure", "Hervé");
            requestXmlTemplate.replace("France", "фгн7");
            QByteArray expectedRequestXml = requestXmlTemplate;
            expectedRequestXml.replace("%1", expectedHeader);
            ret = service.addEmployee(addEmployeeParams);
            QVERIFY(service.lastError().isEmpty());
            QCOMPARE(ret, QString::fromLatin1("Foo"));
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        }

        // Test removing headers
        {
            server.resetReceivedBuffers();
            service.clearLoginHeader();
            service.clearSessionHeader();
            ret = service.addEmployee(addEmployeeParams);
            QByteArray expectedRequestXml = requestXmlTemplate;
            expectedRequestXml.replace("%1", "<soap:Header/>");
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        }
    }

};


QTEST_MAIN(WsdlDocumentTest)

#include "wsdl_document.moc"
