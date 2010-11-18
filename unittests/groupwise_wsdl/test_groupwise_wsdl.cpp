/****************************************************************************
** Copyright (C) 2010-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapAuthentication.h"
#include "wsdl_groupwise.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

#if 0
static const char* xmlEnvBegin =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<soap:Envelope"
        " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
        " xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\""
        " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"";
static const char* xmlEnvEnd = "</soap:Envelope>";
#endif

class GroupwiseTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGeneratedMethods()
    {
        // No runtime test yet, just checking that the methods got generated
        if (false) { // Don't contact localhost:8080 :-)
            GroupwiseService::GroupWiseBinding groupwise;
            METHODS__AcceptRequest acceptRequest;
            acceptRequest.setComment(QString::fromLatin1("Comment"));
            METHODS__AcceptResponse response = groupwise.acceptRequest(acceptRequest);
            (void)response.status();
        }
    }

    // This test uses "internal" API, not very wise.
    // Makes refactoring harder.
    void testStringBaseType()
    {
        TYPES__ContainerRef cref(QString::fromLatin1("str"));
        cref.setDeleted(QDateTime(QDate(2010,31,12)));
        const KDSoapValue v = cref.serialize(QLatin1String("container"));

        TYPES__ContainerRef cref2;
        cref2.deserialize(v);
        QCOMPARE(cref.value(), cref2.value());
        QCOMPARE(cref.deleted(), cref2.deleted());
    }

#if 0
    void testParseComplexReplyWsdl()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        GroupwiseService::GroupWiseBinding groupwise;
        groupwise.setEndPoint(server.endPoint());

        TNS__Query query;
        query.setQueryString(QLatin1String("Select Id, FirstName, LastName from Contact"));
        const TNS__QueryResponse response = sforce.query(query);
        QCOMPARE(sforce.lastError(), QString());
        const TNS__QueryResult result = response.result();
        QVERIFY(result.done());
        QCOMPARE(result.size(), 3);
        QCOMPARE(result.records().size(), 3);
        {
            const ENS__SObject obj = result.records()[0];
            QCOMPARE(obj.id().value(), QLatin1String("01"));
            QCOMPARE(obj.type(), QLatin1String("Contact"));
            const QList<KDSoapValue> anys = obj.any();
            QCOMPARE(anys.size(), 2);
            QCOMPARE(anys[0].name(), QLatin1String("FirstName"));
            QCOMPARE(anys[0].value().toString(), QLatin1String("Kalle"));
            QCOMPARE(anys[1].name(), QLatin1String("LastName"));
            QCOMPARE(anys[1].value().toString(), QLatin1String("Dalheimer"));
        }
        {
            const ENS__SObject obj = result.records()[1];
            QCOMPARE(obj.id().value(), QLatin1String("02"));
            QCOMPARE(obj.type(), QLatin1String("Contact"));
            const QList<KDSoapValue> anys = obj.any();
            QCOMPARE(anys.size(), 2);
            QCOMPARE(anys[0].name(), QLatin1String("FirstName"));
            QCOMPARE(anys[0].value().toString(), QLatin1String("David"));
            QCOMPARE(anys[1].name(), QLatin1String("LastName"));
            QCOMPARE(anys[1].value().toString(), QLatin1String("Faure"));
        }

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) +
            "><soap:Body>"
            "<n1:query xmlns:n1=\"urn:partner.soap.sforce.com\">"
             "<n1:queryString xsi:type=\"xsd:string\">Select Id, FirstName, LastName from Contact</n1:queryString>"
            "</n1:query>"
            "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }
#endif
private:

#if 0
    static QByteArray queryResponse() {
        return QByteArray(xmlEnvBegin) + " xmlns:sf=\"urn:sobject.partner.soap.sforce.com\"><soap:Body>"
              "<queryResponse>"
               "<result xsi:type=\"QueryResult\">"
                "<done>true</done>"
                "<queryLocator xsi:nil=\"true\"/>"
                "<records xsi:type=\"sf:sObject\">"
                  "<sf:type>Contact</sf:type>"
                  "<sf:Id>01</sf:Id><sf:Id>01</sf:Id>"
                  "<sf:FirstName>Kalle</sf:FirstName><sf:LastName>Dalheimer</sf:LastName>"
                "</records>"
                "<records xsi:type=\"sf:sObject\">"
                  "<sf:type>Contact</sf:type>"
                  "<sf:Id>02</sf:Id><sf:Id>02</sf:Id>"
                  "<sf:FirstName>David</sf:FirstName><sf:LastName>Faure</sf:LastName>"
                "</records>"
                "<records xsi:type=\"sf:sObject\">"
                  "<sf:type>Contact</sf:type>"
                  "<sf:Id>03</sf:Id><sf:Id>03</sf:Id>"
                  "<sf:FirstName>Kevin</sf:FirstName><sf:LastName>Krammer</sf:LastName>"
                "</records>"
                "<size>3</size>"
               "</result>"
              "</queryResponse>"
              "</soap:Body>" + xmlEnvEnd;
    }
#endif
};

QTEST_MAIN(GroupwiseTest)

#include "test_groupwise_wsdl.moc"
