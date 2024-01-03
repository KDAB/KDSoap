/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "httpserver_p.h"
#include "wsdl_soap12.h"
#include <QDebug>
#include <QTest>

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

static QByteArray updateObjsResponse()
{
    return QByteArray(xmlEnvBegin11())
        + "><soap:Body>"
          "<createDirectoryResponse xmlns=\"https://www.test.com/testapiv3/testapi.jws\">"
          "</createDirectoryResponse>"
          "</soap:Body>"
        + xmlEnvEnd();
}

void AutoTestSoap12::test()
{
    HttpServerThread server(updateObjsResponse(), HttpServerThread::Public);
    Test::TestSoap12 service12;
    service12.setEndPoint(server.endPoint());

    QCOMPARE(service12.clientInterface()->soapVersion(), KDSoapClientInterface::SOAP1_2);

    service12.version();

    const QByteArray expectedData = QByteArray(xmlEnvBegin11())
        + "><soap:Body>"
          "<n1:version xmlns:n1=\"http://kdab.com/test/\"/>"
          "</soap:Body></soap:Envelope>";

    QByteArray expectedData12 = expectedData;
    expectedData12.replace("http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/2003/05/soap-envelope");
    expectedData12.replace("http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/2003/05/soap-encoding");
    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData12));

    Test::TestSoap service11;
    service11.setEndPoint(server.endPoint());
    QCOMPARE(service11.clientInterface()->soapVersion(), KDSoapClientInterface::SOAP1_1);
    service11.version();
    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData));
}

QTEST_MAIN(AutoTestSoap12)

#include "test_soap12.moc"
