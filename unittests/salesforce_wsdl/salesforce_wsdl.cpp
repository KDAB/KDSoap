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

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapAuthentication.h"
#include "wsdl_salesforce-partner.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

class SalesForceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGeneratedMethods()
    {
        // No runtime test yet, just checking that the methods got generated
        if (false) { // Don't contact www-sjl.salesforce.com :-)
            SforceService sforce;
            TNS__Login loginParams;
            sforce.login(loginParams);
            sforce.logout();

            // Test for the describeLayout-anonymous-complex-type vs DescribeLayout-complex-type conflict
            TNS__DescribeLayoutElement describeParams;
            (void)describeParams.sObjectType();
            TNS__DescribeLayoutResponse describeResponse;
            (void)describeResponse.result().layouts().first().id();
        }

        // Test for the use of reserved C++ keywords such as inline
        TNS__EmailFileAttachment attach;
        attach.setInline(true);
        QVERIFY(attach.inline_());
    }

    void testParseComplexReplyWsdl()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        SforceService sforce(this);
        sforce.setEndPoint(server.endPoint());

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
            QByteArray(xmlEnvBegin11()) +
            "><soap:Body>"
            "<n1:query xmlns:n1=\"urn:partner.soap.sforce.com\">"
             "<n1:queryString>Select Id, FirstName, LastName from Contact</n1:queryString>"
            "</n1:query>"
            "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

private:
    static QByteArray queryResponse() {
        return QByteArray(xmlEnvBegin11()) + " xmlns:sf=\"urn:sobject.partner.soap.sforce.com\"><soap:Body>"
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
              "</soap:Body>" + xmlEnvEnd();
    }
};

QTEST_MAIN(SalesForceTest)

#include "salesforce_wsdl.moc"
