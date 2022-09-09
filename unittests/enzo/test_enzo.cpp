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
    return QByteArray(xmlEnvBegin11())
        + "><soap:Body>"
          // TODO
          "</soap:Body>"
        + xmlEnvEnd();
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
    QObject::connect(job, &AuthenticateJob::finished, &loop, &QEventLoop::quit);
    job->start();
    loop.exec();

    const QByteArray expectedData = QByteArray(xmlEnvBegin11())
        + "><soap:Body>"
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
