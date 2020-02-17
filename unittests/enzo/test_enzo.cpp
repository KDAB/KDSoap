/****************************************************************************
** Copyright (C) 2014-2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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
#include <QTest>
#include <QDebug>
#include "wsdl_EnzoService.h"

class TestEnzo : public QObject
{
    Q_OBJECT
public:
    explicit TestEnzo();

private slots:
    void test();
};

using namespace KDSoapUnitTestHelpers;

TestEnzo::TestEnzo()
{
}

static QByteArray authenticateResponse()
{
    return QByteArray(xmlEnvBegin11()) + "><soap:Body>"
           // TODO
           "</soap:Body>" + xmlEnvEnd();
}

void TestEnzo::test()
{
    HttpServerThread server(authenticateResponse(), HttpServerThread::Public);
    EnzoService service;
    service.setEndPoint(server.endPoint());

    AuthenticateJob *job = new AuthenticateJob(&service);
    I0__Authenticate auth;
    Q2__AuthenticationRequest request;
    request.setHotelCode("hotel");
    request.setPassWord("passWord");
    request.setUserName("user");
    auth.setRequest(request);
    job->setParameters(auth);

    QEventLoop loop;
    QObject::connect(job, SIGNAL(finished(KDSoapJob*)), &loop, SLOT(quit()));
    job->start();
    loop.exec();

    const QByteArray expectedData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
                                    "<n1:Authenticate xmlns:n1=\"http://hotelconcepts.com/\">"
                                     "<n1:request>"
                                      "<n2:HotelCode xmlns:n2=\"http://schemas.datacontract.org/2004/07/HotelConcepts.Web.Enzo.DataContract\">hotel</n2:HotelCode>"
                                      "<n3:PassWord xmlns:n3=\"http://schemas.datacontract.org/2004/07/HotelConcepts.Web.Enzo.DataContract\">passWord</n3:PassWord>"
                                      "<n4:UserName xmlns:n4=\"http://schemas.datacontract.org/2004/07/HotelConcepts.Web.Enzo.DataContract\">user</n4:UserName>"
                                     "</n1:request>"
                                    "</n1:Authenticate></soap:Body></soap:Envelope>";

    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData));
}

QTEST_MAIN(TestEnzo)

#include "test_enzo.moc"
