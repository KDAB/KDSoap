/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2014-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/


#include "httpserver_p.h"
#include "wsdl_uitapi.h"
#include <QDebug>
#include <QEventLoop>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

class UitapiRPCTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Using wsdl-generated code, create a job with an empty response,
    // check that job emits finished (https://github.com/KDAB/KDSoap/issues/94)
    void testEmptyRPCResponse()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<ns1:createDirectoryResponse soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
              "xmlns:ns1=\"https://www.uit.hpc.mil/UITAPIv3/uitapi.jws\"/>"
              " </soap:Body>"
            + xmlEnvEnd();
        HttpServerThread server(responseData, HttpServerThread::Public);

        UitapiService service;
        service.setEndPoint(server.endPoint());

        CreateDirectoryJob *job = new CreateDirectoryJob(&service);
        job->setToken("MyToken");
        job->setHostID(42);
        job->setUsername("me");
        job->setDirectory("dir");

        QEventLoop loop;
        QObject::connect(job, &CreateDirectoryJob::finished, &loop, &QEventLoop::quit);
        job->start();
        loop.exec();
    }

    // Test that RPC replies with a response still work
    void testReturnRPCResponse()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<ns1:getAllocationInfoResponse soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
              "xmlns:ns1=\"https://www.uit.hpc.mil/UITAPIv3/uitapi.jws\"><ns1:getAllocationInfoReturn>The "
              "response</ns1:getAllocationInfoReturn></ns1:getAllocationInfoResponse>"
              " </soap:Body>"
            + xmlEnvEnd();
        HttpServerThread server(responseData, HttpServerThread::Public);

        UitapiService service;
        service.setEndPoint(server.endPoint());

        GetAllocationInfoJob *job = new GetAllocationInfoJob(&service);
        job->setToken("MyToken");
        job->setHostID(42);
        job->setUsername("me");

        QEventLoop loop;
        QObject::connect(job, &GetAllocationInfoJob::finished, &loop, &QEventLoop::quit);
        job->start();
        loop.exec();

        QCOMPARE(job->getAllocationInfoReturn(), QString("The response"));
    }
};


QTEST_MAIN(UitapiRPCTest)

#include "test_uitapi.moc"
