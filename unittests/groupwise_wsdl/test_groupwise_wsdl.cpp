/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDDateTime.h"
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "httpserver_p.h"
#include "wsdl_groupwise.h"
#include <QDebug>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

class GroupwiseTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGeneratedMethods()
    {
        // No runtime test yet, just checking that the methods got generated
        if (false) { // Don't contact localhost:8080 :-)
            GroupwiseService::GroupWiseBinding groupwise;
            groupwise.setGwTraceHeader(true);
            METHODS__AcceptRequest acceptRequest;
            acceptRequest.setComment(QString::fromLatin1("Comment"));
            METHODS__AcceptResponse response = groupwise.acceptRequest(acceptRequest);
            ( void )response.status();

            GroupwiseService::GroupWiseEventsBinding groupwiseEvents;
            groupwiseEvents.setGwTraceHeader(true);
        }
    }

    // Note: this test uses "internal" API, serialize/deserialize... not a good example.
    void testStringBaseType()
    {
        TYPES__ContainerRef cref(QString::fromLatin1("str"));
        cref.setDeleted(QDateTime(QDate(2010, 12, 31), QTime(1, 2, 3))); // implicit conversion from QDateTime to KDDateTime
        const KDSoapValue v = cref.serialize(QLatin1String("container"));

        // qDebug() << v.toXml();

        TYPES__ContainerRef cref2;
        cref2.deserialize(v);
        QCOMPARE(cref.value(), cref2.value());
        QCOMPARE(cref.deleted(), cref2.deleted());
    }

    void testBase64()
    {
        HttpServerThread server(updateVersionStatusResponse(), HttpServerThread::Public);
        GroupwiseService::GroupWiseBinding groupwise;
        groupwise.setEndPoint(server.endPoint());

        METHODS__UpdateVersionStatusRequest req;
        req.setEvent(TYPES__VersionEventType::Archive);
        req.setId(QString::fromLatin1("TheId"));
        TYPES__SignatureData sigData;
        sigData.setData("ABCDEF");
        sigData.setSize(6);
        req.setPart(sigData);
        METHODS__UpdateVersionStatusResponse response = groupwise.updateVersionStatusRequest(req);

        // Check what we sent
        // This WSDL (well, the XSD) uses elementFormDefault="qualified", so we get namespace definitions.
        // TODO: declare them up in the main element, to make this shorter
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<n1:updateVersionStatusRequest xmlns:n1=\"http://schemas.novell.com/2005/01/GroupWise/methods\">"
              "<n1:id>TheId</n1:id>"
              "<n1:event>archive</n1:event>"
              "<n1:part>"
              "<n2:size xmlns:n2=\"http://schemas.novell.com/2005/01/GroupWise/types\">6</n2:size>"
              "<n3:data xmlns:n3=\"http://schemas.novell.com/2005/01/GroupWise/types\">QUJDREVG</n3:data>"
              "</n1:part>"
              "</n1:updateVersionStatusRequest>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        // The response didn't have the expected fields. Not sure how we should handle that.
        // Right now there's no error, just an "empty" response data.

        QCOMPARE(response.status().code(), 0);
    }

    void testDateTimeInRequest()
    {
        HttpServerThread server(updateVersionStatusResponse(), HttpServerThread::Public);
        GroupwiseService::GroupWiseBinding groupwise;
        groupwise.setEndPoint(server.endPoint());

        METHODS__SetTimestampRequest req;
        KDDateTime frenchDate(QDateTime(QDate(2011, 03, 15), QTime(4, 3, 2, 1)));
        frenchDate.setTimeZone(QString::fromLatin1("+01:00"));
        req.setBackup(frenchDate);

        // implicit conversion from KDDateTime to QDateTime
        req.setRetention(QDateTime(QDate(2011, 01, 15), QTime(4, 3, 2, 1)));

        /*METHODS__SetTimestampResponse response = */ groupwise.setTimestampRequest(req);

        // Check what we sent
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<n1:setTimestampRequest xmlns:n1=\"http://schemas.novell.com/2005/01/GroupWise/methods\">"
              "<n1:backup>2011-03-15T04:03:02.001+01:00</n1:backup>"
              "<n1:retention>2011-01-15T04:03:02.001</n1:retention>"
              "</n1:setTimestampRequest>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testDateTimeInResponse()
    {
        HttpServerThread server(getTimestampResponse(), HttpServerThread::Public);
        GroupwiseService::GroupWiseBinding groupwise;
        groupwise.setEndPoint(server.endPoint());

        METHODS__GetTimestampRequest req;
        req.setBackup(true);
        req.setRetention(true);
        METHODS__GetTimestampResponse response = groupwise.getTimestampRequest(req);

        // Check what we sent
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<n1:getTimestampRequest xmlns:n1=\"http://schemas.novell.com/2005/01/GroupWise/methods\">"
              "<n1:backup>true</n1:backup>"
              "<n1:retention>true</n1:retention>"
              "</n1:getTimestampRequest>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        // Check response parsing
        QCOMPARE(response.backup().toString(Qt::ISODate), QString::fromLatin1("2011-03-15T04:03:02+01:00")); // Qt doesn't show msec
        QCOMPARE(response.backup().timeZone(), QString::fromLatin1("+01:00"));
        QCOMPARE(response.backup().toDateString(), QString::fromLatin1("2011-03-15T04:03:02.001+01:00"));
        QCOMPARE(response.retention().toDateString(), QString::fromLatin1("2011-01-15T04:03:02.001Z"));
        QCOMPARE(response.retention().timeZone(), QString::fromLatin1("Z"));
    }

private:
    // Bogus response
    static QByteArray updateVersionStatusResponse()
    {
        return QByteArray(xmlEnvBegin11())
            + " xmlns:gw=\"http://schemas.novell.com/2005/01/GroupWise/groupwise.wsdl\"><soap:Body>"
              "<queryResponse>"
              "<result>"
              "<done>true</done>"
              "<size>3</size>"
              "</result>"
              "</queryResponse>"
              "</soap:Body>"
            + xmlEnvEnd();
    }

    static QByteArray getTimestampResponse()
    {
        return QByteArray(xmlEnvBegin11())
            + " xmlns:gw=\"http://schemas.novell.com/2005/01/GroupWise/groupwise.wsdl\"><soap:Body>"
              "<gw:getTimestampResponse>"
              "<gw:backup>2011-03-15T04:03:02.001+01:00</gw:backup>"
              "<gw:retention>2011-01-15T04:03:02.001Z</gw:retention>"
              "</gw:getTimestampResponse>"
              "</soap:Body>"
            + xmlEnvEnd();
    }
};

QTEST_MAIN(GroupwiseTest)

#include "test_groupwise_wsdl.moc"
