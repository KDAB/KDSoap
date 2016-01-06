/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
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

#include "wsdl_transformMedia-V1_0_7.h"

#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapServer.h>
#include <KDSoapNamespaceManager.h>

using namespace KDSoapUnitTestHelpers;

// Server side to perform job operation request
class TransformMediaBindingServerObject : public TransformMediaBindingServerBase /* generated from wsdl */
{
    Q_OBJECT
public:
    virtual TFMS__TransformResponseType transform(const TFMS__TransformRequestType &in)
    {
        TFMS__TransformJobType copyJob = in.transformJob();
        Q_ASSERT(copyJob.operationName() == "operation");
        BMS__QueueType queueType;
        queueType.setLength(10);
        copyJob.setQueueReference(queueType);
        TFMS__TransformResponseType response;
        response.setTransformJob(copyJob);
        Q_ASSERT(response.transformJob().operationName() == "operation");
        return response;
    }
};

class TransformMediaBindingServer : public KDSoapServer
{
    Q_OBJECT
public:
    TransformMediaBindingServer() : KDSoapServer(), m_lastServerObject(0)
    {
        setPath(QLatin1String("/xml"));
    }
    virtual QObject *createServerObject()
    {
        m_lastServerObject = new TransformMediaBindingServerObject;
        return m_lastServerObject;
    }

    TransformMediaBindingServerObject *lastServerObject()
    {
        return m_lastServerObject;
    }

private:
    TransformMediaBindingServerObject *m_lastServerObject; // only for unittest purposes
};

// Server side to perform job status request
class TransformMediaStatusBindingServerObject : public TransformMediaStatusBindingServerBase /* generated from wsdl */
{
    Q_OBJECT
public:
    virtual BMS__ManageJobResponseType manageJob(const BMS__ManageJobRequestType &in)
    {
        // check what was sent
        Q_UNUSED(in);
        if (in.jobID().value().toInt() != 1) {
            // use faults when implemented !
            return BMS__ManageJobResponseType();
        }
        Q_ASSERT(in.jobCommand().type() == BMS__JobCommandType::Restart);
        // prepare response containing a job status
        BMS__ManageJobResponseType response;
        BMS__JobType myJob;
        myJob.setStatus(BMS__JobStatusType::Running);
        response.setJob(myJob);
        Q_ASSERT(response.job().status().type() == BMS__JobStatusType::Running);
        return response;
    }
    virtual BMS__ManageQueueResponseType manageQueue(const BMS__ManageQueueRequestType &in)
    {
        Q_UNUSED(in);
        BMS__ManageQueueResponseType qrt;
        return qrt;
    }
    virtual BMS__QueryJobResponseType queryJob(const BMS__QueryJobRequestType &in)
    {
        Q_UNUSED(in);
        BMS__QueryJobResponseType jrt;
        return jrt;
    }
};

class TransformMediaStatusBindingServer : public KDSoapServer
{
    Q_OBJECT
public:
    TransformMediaStatusBindingServer() : KDSoapServer(), m_lastServerObject(0)
    {
        setPath(QLatin1String("/xml"));
    }
    virtual QObject *createServerObject()
    {
        m_lastServerObject = new TransformMediaStatusBindingServerObject;
        return m_lastServerObject;
    }

    TransformMediaStatusBindingServerObject *lastServerObject()
    {
        return m_lastServerObject;
    }

private:
    TransformMediaStatusBindingServerObject *m_lastServerObject; // only for unittest purposes
};

class Tech3356Test : public QObject
{
    Q_OBJECT

private:

    static QByteArray expectedQueryJobRequest()
    {
        return QByteArray(xmlEnvBegin11()) + ">"
               "<soap:Body>"
               "<n1:queryJobRequest xmlns:n1=\"http://base.fims.tv\">" // MISSING: xsi:type=\"n1:QueryJobRequestByIDType\">"
               "<n1:jobInfoSelection>all</n1:jobInfoSelection>"
               "<n1:jobID>1</n1:jobID>"
               "</n1:queryJobRequest>"
               "</soap:Body>" + xmlEnvEnd()
               + '\n'; // added by QXmlStreamWriter::writeEndDocument;
    }

    static QByteArray queryJobResponse()
    {
        return "<?xml version='1.0' encoding='UTF-8'?>"
               "<SOAP-ENV:Envelope "
               "xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
               "xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\" "
               "xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\">"
               "<SOAP-ENV:Body>"
               "<ns1:sayHelloResponse "
               "xmlns:ns1=\"urn:examples:helloservice\" "
               "SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
               "<return xsi:type=\"xsd:string\">Hello, World!</return>"
               "</ns1:sayHelloResponse>"
               "</SOAP-ENV:Body>"
               "</SOAP-ENV:Envelope>";
    }

private Q_SLOTS:

    void testQueryJob()
    {
        HttpServerThread server(queryJobResponse(), HttpServerThread::Public);
        TransformMediaService::TransformMediaStatusBinding service;
        service.setEndPoint(server.endPoint());

        // check that <sequence minOccurs="1" maxOccurs="unbounded"> actually created a QList.
        BMS__JobsType jobsType;
        BMS__JobType jobType1;
        BMS__JobType jobType2;
        jobsType.setJob(QList<BMS__JobType>() << jobType1 << jobType2);

        // check that derived types are defined (i.e. don't get cleaned up as unused)
        BMS__QueryJobRequestByIDType req;
        BMS__UID uid; uid.setValue(1);
        req.setJobID(QList<BMS__UID>() << uid);
        req.setJobInfoSelection(BMS__JobInfoSelectionType::All); // setter in base class

        // check the operator= and copy ctor of derived types
        BMS__QueryJobRequestByIDType reqCopy(req);
        QCOMPARE(reqCopy.jobID().count(), 1);
        QCOMPARE(int(reqCopy.jobInfoSelection()), int(req.jobInfoSelection()));
        reqCopy = req;
        BMS__QueryJobRequestByIDType reqCopy2;
        reqCopy2 = req;
        QCOMPARE(reqCopy2.jobID().count(), 1);
        QCOMPARE(int(reqCopy2.jobInfoSelection()), int(req.jobInfoSelection()));

        // check that <element name="processed" type="bms:ProcessedInfoType" minOccurs="0">
        // leads to a method that returns a pointer, rather than a const ref (which would be a null ref then, when not set...)
        const BMS__ProcessedInfoType *proc = jobType1.processed();
        QCOMPARE(proc, static_cast<BMS__ProcessedInfoType *>(0));

        // check that we can pass a derived class where a base class is expected
        BMS__QueryJobResponseType respType = service.queryJob(req);

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedQueryJobRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedQueryJobRequest().constData()));
        }
    }

    void testTransformJob() // SOAP-80
    {
        TestServerThread<TransformMediaBindingServer> serverThread;
        TransformMediaBindingServer *server = serverThread.startThread();

        TransformMediaService::TransformMediaBinding service;
        service.setEndPoint(server->endPoint());

        TFMS__TransformJobType transformJob;
        transformJob.setOperationName(QLatin1String("operation"));

        TFMS__TransformRequestType request;
        request.setTransformJob(transformJob);
        TFMS__TransformResponseType response = service.transform(request);
        QCOMPARE(response.transformJob().operationName(), QString::fromLatin1("operation"));
    }

    void testJobStatusRequest() // SOAP-80
    {
        TestServerThread<TransformMediaStatusBindingServer> serverThread;
        TransformMediaStatusBindingServer *server = serverThread.startThread();

        TransformMediaService::TransformMediaStatusBinding service;
        service.setEndPoint(server->endPoint());

        BMS__ManageJobRequestType requestType;
        // mandatory ID
        BMS__UID uid; uid.setValue(1);
        requestType.setJobID(uid);
        BMS__ExtensionAttributes ext;
        // mandatory JobCommand
        BMS__JobCommandType jt;
        jt.setType(BMS__JobCommandType::Restart);
        requestType.setJobCommand(jt);

        BMS__ManageJobResponseType myResp = service.manageJob(requestType);
        QCOMPARE(myResp.job().status().type(), BMS__JobStatusType::Running);
    }
};

QTEST_MAIN(Tech3356Test)

#include "test_tech3356.moc"
