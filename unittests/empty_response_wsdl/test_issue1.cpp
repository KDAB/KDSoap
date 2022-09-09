/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "httpserver_p.h"
#include "wsdl_test.h"
#include <QDebug>
#include <QTest>

class TestIssue1 : public QObject
{
    Q_OBJECT
public:
    explicit TestIssue1();

private slots:
    void test();
};

using namespace KDSoapUnitTestHelpers;

TestIssue1::TestIssue1()
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

void TestIssue1::test()
{
    HttpServerThread server(updateObjsResponse(), HttpServerThread::Public);
    TestapiService service;
    service.setEndPoint(server.endPoint());

    service.createDirectory("t", 42, "user", "dir");

    const QByteArray expectedData = QByteArray(xmlEnvBegin11())
        + "><soap:Body>"
          "<n1:createDirectory xmlns:n1=\"http://DefaultNamespace\">"
          "<token xsi:type=\"xsd:string\">t</token>"
          "<hostID xsi:type=\"xsd:long\">42</hostID>"
          "<username xsi:type=\"xsd:string\">user</username>"
          "<directory xsi:type=\"xsd:string\">dir</directory>"
          "</n1:createDirectory></soap:Body></soap:Envelope>";

    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData));
}

QTEST_MAIN(TestIssue1)

#include "test_issue1.moc"
