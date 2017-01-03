/****************************************************************************
** Copyright (C) 2014-2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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


#include "wsdl_uitapi.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

class UitapiRPCTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Using wsdl-generated code, create a job with an empty response,
    // check that job emits finnished (https://github.com/KDAB/KDSoap/issues/94)
    void testEmptyRPCResponse()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
                                  "<ns1:createDirectoryResponse soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:ns1=\"https://www.uit.hpc.mil/UITAPIv3/uitapi.jws\"/>"
                                  " </soap:Body>" + xmlEnvEnd();
        HttpServerThread server(responseData, HttpServerThread::Public);

        UitapiService service;
        service.setEndPoint(server.endPoint());

        CreateDirectoryJob *job = new CreateDirectoryJob(&service);
        job->setToken("MyToken");
        job->setHostID(42);
        job->setUsername("me");
        job->setDirectory("dir");

        QEventLoop loop;
        QObject::connect(job, SIGNAL(finished(KDSoapJob*)), &loop, SLOT(quit()));
        job->start();
        loop.exec();
    }

    // Test that RPC replies with a response still work
    void testReturnRPCResponse()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
            "<ns1:getAllocationInfoResponse soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:ns1=\"https://www.uit.hpc.mil/UITAPIv3/uitapi.jws\"><ns1:getAllocationInfoReturn>The response</ns1:getAllocationInfoReturn></ns1:getAllocationInfoResponse>"
                                              " </soap:Body>" + xmlEnvEnd();
        HttpServerThread server(responseData, HttpServerThread::Public);

        UitapiService service;
        service.setEndPoint(server.endPoint());

        GetAllocationInfoJob *job = new GetAllocationInfoJob(&service);
        job->setToken("MyToken");
        job->setHostID(42);
        job->setUsername("me");

        QEventLoop loop;
        QObject::connect(job, SIGNAL(finished(KDSoapJob*)), &loop, SLOT(quit()));
        job->start();
        loop.exec();

        QCOMPARE(job->getAllocationInfoReturn(), QString("The response"));
    }
};


QTEST_MAIN(UitapiRPCTest)

#include "test_uitapi.moc"



