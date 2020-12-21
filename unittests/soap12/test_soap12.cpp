/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2014-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/

#include "httpserver_p.h"
#include <QTest>
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

static QByteArray updateObjsResponse()
{
    return QByteArray(xmlEnvBegin11()) + "><soap:Body>"
           "<createDirectoryResponse xmlns=\"https://www.test.com/testapiv3/testapi.jws\">"
           "</createDirectoryResponse>"
           "</soap:Body>" + xmlEnvEnd();
}

void AutoTestSoap12::test()
{
    HttpServerThread server(updateObjsResponse(), HttpServerThread::Public);
    Test::TestSoap12 service12;
    service12.setEndPoint(server.endPoint());

    QCOMPARE(service12.clientInterface()->soapVersion(), KDSoapClientInterface::SOAP1_2);

    service12.version();

    const QByteArray expectedData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
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
