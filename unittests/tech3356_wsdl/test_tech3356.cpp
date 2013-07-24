/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
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

        // check that we can pass a derived class where a base class is expected
        BMS__QueryJobResponseType respType = service.queryJob(req);

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedQueryJobRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedQueryJobRequest().constData()));
        }
    }
};

QTEST_MAIN(Tech3356Test)

#include "test_tech3356.moc"
