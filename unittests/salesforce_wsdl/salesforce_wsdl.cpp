#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapAuthentication.h"
#include "wsdl_salesforce-partner.h"
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

class SalesForceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGeneratedMethods()
    {
        // No runtime test yet, just checking that the methods got generated
        if (false) { // Don't contact www-sjl.salesforce.com :-)
            SforceService sforce;
            TNS__Login loginParams;
            sforce.login(loginParams);
            sforce.logout();
        }
    }

    void testParseComplexReplyWsdl()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        SforceService sforce(this);
        sforce.setEndPoint(server.endPoint());

        TNS__Query query;
        query.setQueryString(QLatin1String("Select Id, FirstName, LastName from Contact"));
        const TNS__QueryResponse response = sforce.query(query);
        QCOMPARE(sforce.lastError(), QString());
        const TNS__QueryResult result = response.result();
        QVERIFY(result.done());
        QCOMPARE(result.size(), 3);
        QCOMPARE(result.records().size(), 3);
        const ENS__SObject obj1 = result.records()[0];
        QCOMPARE(obj1.id().value(), QLatin1String("01"));
        QCOMPARE(obj1.type(), QLatin1String("Contact"));
#if 0 // TODO
        QCOMPARE(obj1.any()[0], QLatin1String("Kalle")); // TODO KDSoapMessage
#endif
        // TODO finish

#if 0
        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) +
            "><soap:Body>"
            // TODO?
            "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
#endif
    }

private:
    static QByteArray queryResponse() {
        return QByteArray(xmlEnvBegin) + " xmlns:sf=\"urn:sobject.partner.soap.sforce.com\"><soap:Body>"
              "<queryResponse>"
               "<result xsi:type=\"QueryResult\">"
                "<done>true</done>"
                "<queryLocator xsi:nil=\"true\"/>"
                "<records xsi:type=\"sf:sObject\">"
                  "<sf:type>Contact</sf:type>"
                  "<sf:Id>01</sf:Id><sf:Id>01</sf:Id>"
                  "<sf:FirstName>Kalle</sf:FirstName><sf:LastName>Dalheimer</sf:LastName>"
                "</records>"
                "<records xsi:type=\"sf:sObject\">"
                  "<sf:type>Contact</sf:type>"
                  "<sf:Id>02</sf:Id><sf:Id>02</sf:Id>"
                  "<sf:FirstName>David</sf:FirstName><sf:LastName>Faure</sf:LastName>"
                "</records>"
                "<records xsi:type=\"sf:sObject\">"
                  "<sf:type>Contact</sf:type>"
                  "<sf:Id>03</sf:Id><sf:Id>03</sf:Id>"
                  "<sf:FirstName>Kevin</sf:FirstName><sf:LastName>Krammer</sf:LastName>"
                "</records>"
                "<size>3</size>"
               "</result>"
              "</queryResponse>"
              "</soap:Body>" + xmlEnvEnd;
    }
};

QTEST_MAIN(SalesForceTest)

#include "salesforce_wsdl.moc"
