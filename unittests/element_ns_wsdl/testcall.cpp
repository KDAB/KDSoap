/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "testcall.h"
#include "httpserver_p.h"
#include <QTest>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

TestCall::TestCall()
{
}

static QByteArray updateObjsResponse()
{
    return QByteArray(xmlEnvBegin11())
        + "><soap:Body>"
          "<updateObjResponseElement xmlns=\"http://test.example.com/types/\">"
          "</updateObjResponseElement>"
          "</soap:Body>"
        + xmlEnvEnd();
}

void TestCall::test()
{
    HttpServerThread server(updateObjsResponse(), HttpServerThread::Public);
    this->test_client.setEndPoint(server.endPoint());

    TNS0__UpdateObjElement obj_el;
    QList<TNS0__MyObj> objs;
    TNS0__MyObj t;
    t.setField1("test1");
    t.setId("1");
    objs.append(t);
    TNS0__Credential credential;
    credential.setId("id");
    credential.setPassWord("password");
    obj_el.setArrayOfObj(objs);
    obj_el.setCredential(credential);
    this->test_client.updateObjs(obj_el);

    const QByteArray expectedData = QByteArray(xmlEnvBegin11())
        + "><soap:Body>"
          "<n1:updateObjElement xmlns:n1=\"http://test.example.com/types/\">"
          "<n1:arrayOfObj>"
          "<n1:Id>1</n1:Id>"
          "<n1:Field1>test1</n1:Field1>"
          "</n1:arrayOfObj>"
          "<n1:credential>"
          "<n1:Id>id</n1:Id>"
          "<n1:PassWord>password</n1:PassWord>"
          "</n1:credential>"
          "</n1:updateObjElement></soap:Body></soap:Envelope>";

    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData));
}

QTEST_MAIN(TestCall)
