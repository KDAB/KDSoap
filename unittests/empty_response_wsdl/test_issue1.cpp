/****************************************************************************
** Copyright (C) 2014-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QDebug>
#include "wsdl_test.h"

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
    return QByteArray(xmlEnvBegin11()) + "><soap:Body>"
           "<createDirectoryResponse xmlns=\"https://www.test.com/testapiv3/testapi.jws\">"
           "</createDirectoryResponse>"
           "</soap:Body>" + xmlEnvEnd();
}

void TestIssue1::test()
{
    HttpServerThread server(updateObjsResponse(), HttpServerThread::Public);
    TestapiService service;
    service.setEndPoint(server.endPoint());

    service.createDirectory("t", 42, "user", "dir");

    const QByteArray expectedData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
                                    "<n1:createDirectory xmlns:n1=\"https://www.test.com/testapiv3/testapi.jws\">"
                                    "<token xsi:type=\"xsd:string\">t</token>"
                                    "<hostID xsi:type=\"xsd:long\">42</hostID>"
                                    "<username xsi:type=\"xsd:string\">user</username>"
                                    "<directory xsi:type=\"xsd:string\">dir</directory>"
                                    "</n1:createDirectory></soap:Body></soap:Envelope>";

    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData));
}

QTEST_MAIN(TestIssue1)

#include "test_issue1.moc"
