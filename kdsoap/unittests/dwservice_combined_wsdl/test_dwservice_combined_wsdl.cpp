#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapAuthentication.h"
#include "KDDateTime.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QDebug>

// Testcase for multiple services defined in one .wsdl file
#include "wsdl_DWServiceCombined.h"

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

class DWServiceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testLogin()
    {
        HttpServerThread server(loginResponse(), HttpServerThread::Public);
        KDAB::DWService service;
        service.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        service.setEndPoint(server.endPoint());

        KDAB::TNS__Login loginParams;
        loginParams.setUserName(QString::fromLatin1("Foo"));
        loginParams.setPassword(QString::fromLatin1("Bar"));
        loginParams.setOrganization(QString::fromLatin1("KDAB"));
        const KDAB::TNS__LoginResponse resp = service.login(loginParams);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) +
            "><soap:Body>"
            "<n1:Login xmlns:n1=\"http://tempuri.org/\">"
                "<n1:userName>Foo</n1:userName>"
                "<n1:password>Bar</n1:password>"
                "<n1:organization>KDAB</n1:organization>"
            "</n1:Login>"
            "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        const KDAB::__ClientServiceSession session = resp.loginResult();
        const KDAB::__SessionBase sessionBase = session.value();
        const KDAB::__Guid sessionId = sessionBase.sessionID();
        QCOMPARE(sessionId.value(), QString::fromLatin1("65a65c1f-2613-47d0-89ec-1c7b1fe34777"));
    }

    void testLogoff()
    {
        HttpServerThread server(loginResponse(), HttpServerThread::Public);
        KDAB::DWService service;
        service.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        service.setEndPoint(server.endPoint());

        KDAB::__SessionBase sessionBase;
        sessionBase.setSessionID(KDAB::__Guid(QString::fromLatin1("65a65c1f-2613-47d0-89ec-1c7b1fe34777")));
        KDAB::__ClientServiceSession session(sessionBase);
        KDAB::TNS__Logoff logoffParams;
        logoffParams.setClientSession(session);
        service.logoff(logoffParams);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) +
            "><soap:Body>"
            "<n1:Logoff xmlns:n1=\"http://tempuri.org/\">"
              "<n1:clientSession><n2:SessionID xmlns:n2=\"http://schemas.datacontract.org/2004/07/DocuWare.WebServices.GAPIFunctionality.DataContracts\">65a65c1f-2613-47d0-89ec-1c7b1fe34777</n2:SessionID></n1:clientSession>"
            "</n1:Logoff>"
            "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

    }

private:
    static QByteArray loginResponse()
    {
        return QByteArray(xmlEnvBegin) + " xmlns:dw=\"http://schemas.novell.com/2005/01/GroupWise/groupwise.wsdl\"><soap:Body>"
              "<dw:LoginResponse>"
                "<dw:LoginResult><dw:SessionID>65a65c1f-2613-47d0-89ec-1c7b1fe34777</dw:SessionID></dw:LoginResult>"
               "</dw:LoginResponse>"
              "</soap:Body>" + xmlEnvEnd;
    }

};

QTEST_MAIN(DWServiceTest)

#include "test_dwservice_combined_wsdl.moc"
