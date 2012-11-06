#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QDebug>
#include "wsdl_soap12.h"

class AutoTestSoap12 : public QObject
{
    Q_OBJECT
public:
    explicit AutoTestSoap12();

private slots:
    void test();
};

using namespace KDSoapUnitTestHelpers;

AutoTestSoap12::AutoTestSoap12()
{
}

static const char* xmlEnvBegin =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<soap:Envelope"
        " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
        " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"";
static const char* xmlEnvEnd = "</soap:Envelope>";

static QByteArray updateObjsResponse() {
    return QByteArray(xmlEnvBegin) + "><soap:Body>"
            "<createDirectoryResponse xmlns=\"https://www.test.com/testapiv3/testapi.jws\">"
            "</createDirectoryResponse>"
            "</soap:Body>" + xmlEnvEnd;
}

void AutoTestSoap12::test()
{
    HttpServerThread server(updateObjsResponse(), HttpServerThread::Public);
    Test::TestSoap12 service12;
    service12.setEndPoint(server.endPoint());

    QCOMPARE(service12.clientInterface()->soapVersion(), KDSoapClientInterface::SOAP1_2);

    service12.version();

    const QByteArray expectedData = QByteArray(xmlEnvBegin) + "><soap:Body>"
    "<n1:version xmlns:n1=\"http://kdab.com/test/\"/>"
    "</soap:Body></soap:Envelope>";

    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData));

    Test::TestSoap service11;
    service11.setEndPoint(server.endPoint());
    QCOMPARE(service11.clientInterface()->soapVersion(), KDSoapClientInterface::SOAP1_1);
    service11.version();
    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData));
}

QTEST_MAIN(AutoTestSoap12)

#include "test_soap12.moc"
