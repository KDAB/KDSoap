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

#include "testcall.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

TestCall::TestCall()
{
}

static QByteArray updateObjsResponse()
{
    return QByteArray(xmlEnvBegin11()) + "><soap:Body>"
           "<updateObjResponseElement xmlns=\"http://test.example.com/types/\">"
           "</updateObjResponseElement>"
           "</soap:Body>" + xmlEnvEnd();
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

    const QByteArray expectedData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
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
