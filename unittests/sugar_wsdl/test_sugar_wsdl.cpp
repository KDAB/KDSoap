/****************************************************************************
** Copyright (C) 2010-2012 Klaralvdalens Datakonsult AB.  All rights reserved.
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

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "wsdl_sugarcrm.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

Q_DECLARE_METATYPE(TNS__Set_entry_result)

using namespace KDSoapUnitTestHelpers;

static const char* xmlEnvBegin =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<soap:Envelope"
        " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
        " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"";
static const char* xmlEnvEnd = "</soap:Envelope>";

class SugarTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testParseComplexReplyWsdl()
    {
        HttpServerThread server(complexTypeResponse(), HttpServerThread::Public);
        Sugarsoap sugar(this);
        sugar.setEndPoint(server.endPoint());
        TNS__User_auth user_auth;
        user_auth.setUser_name(QString::fromUtf8("user å"));
        user_auth.setPassword(QString::fromLatin1("pass"));
        TNS__Set_entry_result result = sugar.login(user_auth, QString::fromLatin1("application"));
        QCOMPARE(result.id(), QString::fromLatin1("12345"));
        QCOMPARE(result.error().number(), QString::fromLatin1("0"));
        QCOMPARE(result.error().name(), QString::fromLatin1("No Error"));
        QCOMPARE(result.error().description(), QString::fromLatin1("No Error"));

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) +
            "><soap:Body>"
            "<n1:login xmlns:n1=\"http://www.sugarcrm.com/sugarcrm\">"
              "<user_auth xsi:type=\"n1:user_auth\">"
                "<user_name xsi:type=\"xsd:string\">user å</user_name>"
                "<password xsi:type=\"xsd:string\">pass</password>"
                "<version xsi:type=\"xsd:string\" xsi:nil=\"true\"></version>"
              "</user_auth>"
              "<application_name xsi:type=\"xsd:string\">application</application_name>"
            "</n1:login>"
            "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testParseComplexReplyWsdlAsync()
    {
        HttpServerThread server(complexTypeResponse(), HttpServerThread::Public);
        Sugarsoap sugar(this);
        sugar.setEndPoint(server.endPoint());
        TNS__User_auth user_auth;
        user_auth.setUser_name(QString::fromLatin1("user"));
        user_auth.setPassword(QString::fromLatin1("pass"));
        qRegisterMetaType<TNS__Set_entry_result>("TNS__Set_entry_result");
        QSignalSpy loginDoneSpy(&sugar, SIGNAL(loginDone(TNS__Set_entry_result)));
        sugar.asyncLogin(user_auth, QString::fromLatin1("application"));
        QEventLoop loop;
        connect(&sugar, SIGNAL(loginDone(TNS__Set_entry_result)), &loop, SLOT(quit()));
        loop.exec();
        const TNS__Set_entry_result result = loginDoneSpy[0][0].value<TNS__Set_entry_result>();
        QCOMPARE(result.id(), QString::fromLatin1("12345"));
        QCOMPARE(result.error().number(), QString::fromLatin1("0"));
        QCOMPARE(result.error().name(), QString::fromLatin1("No Error"));
        QCOMPARE(result.error().description(), QString::fromLatin1("No Error"));
    }

    void testParseReplyWithArray()
    {
        HttpServerThread server(arrayResponse(), HttpServerThread::Public);
        Sugarsoap sugar(this);
        sugar.setEndPoint(server.endPoint());

        const TNS__Module_list modules = sugar.get_available_modules(QLatin1String("session"));
        QCOMPARE(sugar.lastError(), QString());
        const TNS__Select_fields fields = modules.modules();
        const QStringList items = fields.items();
        QCOMPARE(items.count(), 4);
        // Check that it's a QStringList, not a QList<QString>
        const QString itemsJoined = fields.items().join(QString(QLatin1Char(',')));
        QCOMPARE(itemsJoined, QString::fromLatin1("Home,Dashboard,Calendar,Activities"));
    }

    static QByteArray arrayResponse() {
        return QByteArray(xmlEnvBegin) + " xmlns:tns=\"http://www.sugarcrm.com/sugarcrm\"><soap:Body>"
                "<ns1:get_available_modulesResponse xmlns:ns1=\"http://www.sugarcrm.com/sugarcrm\">"
                "<return xsi:type=\"tns:module_list\">"
                "<modules xsi:type=\"soap-enc:Array\" soap-enc:arrayType=\"xsd:string[4]\">"
                "<item xsi:type=\"xsd:string\">Home</item>"
                "<item xsi:type=\"xsd:string\">Dashboard</item>"
                "<item xsi:type=\"xsd:string\">Calendar</item>"
                "<item xsi:type=\"xsd:string\">Activities</item>"
                "</modules>"
                "<error xsi:type=\"tns:error_value\">"
                "<number xsi:type=\"xsd:string\">0</number>"
                "<name xsi:type=\"xsd:string\">No Error</name>"
                "<description xsi:type=\"xsd:string\">No Error</description>"
                "</error>"
                "</return>"
                "</ns1:get_available_modulesResponse>"
                "</soap:Body>" + xmlEnvEnd;
    }


private:
    static QByteArray complexTypeResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body xmlns:tns=\"http://www.sugarcrm.com/sugarcrm\">"
                "<ns1:loginResponse xmlns:ns1=\"http://www.sugarcrm.com/sugarcrm\">"
                "  <return xsi:type=\"tns:set_entry_result\">"
                "    <id xsi:type=\"xsd:string\">12345</id>"
                "    <error xsi:type=\"tns:error_value\">"
                "       <number xsi:type=\"xsd:string\">0</number>"
                "       <name xsi:type=\"xsd:string\">No Error</name>"
                "       <description xsi:type=\"xsd:string\">No Error</description>"
                "    </error>"
                "  </return>"
                "</ns1:loginResponse>"
                "</soap:Body>" + xmlEnvEnd;
    }
};

QTEST_MAIN(SugarTest)

#include "test_sugar_wsdl.moc"
