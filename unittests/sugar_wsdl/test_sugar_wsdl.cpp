/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "httpserver_p.h"
#include "wsdl_sugarcrm.h"
#include <QDebug>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(TNS__Set_entry_result)

using namespace KDSoapUnitTestHelpers;

class SugarTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

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
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<n1:login xmlns:n1=\"http://www.sugarcrm.com/sugarcrm\">"
              "<user_auth xsi:type=\"n1:user_auth\">"
              "<user_name xsi:type=\"xsd:string\">user å</user_name>"
              "<password xsi:type=\"xsd:string\">pass</password>"
              "<version xsi:type=\"xsd:string\"/>" // not set by user, but mandatory (no minOccurs==0)
              "</user_auth>"
              "<application_name xsi:type=\"xsd:string\">application</application_name>"
              "</n1:login>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
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
        QSignalSpy loginDoneSpy(&sugar, &Sugarsoap::loginDone);
        sugar.asyncLogin(user_auth, QString::fromLatin1("application"));
        QEventLoop loop;
        connect(&sugar, &Sugarsoap::loginDone, &loop, &QEventLoop::quit);
        loop.exec();
        const TNS__Set_entry_result result = loginDoneSpy[0][0].value<TNS__Set_entry_result>();
        QCOMPARE(result.id(), QString::fromLatin1("12345"));
        QCOMPARE(result.error().number(), QString::fromLatin1("0"));
        QCOMPARE(result.error().name(), QString::fromLatin1("No Error"));
        QCOMPARE(result.error().description(), QString::fromLatin1("No Error"));
    }

    void testParseComplexReplyWsdlJob()
    {
        HttpServerThread server(complexTypeResponse(), HttpServerThread::Public);
        Sugarsoap sugar(this);
        sugar.setEndPoint(server.endPoint());
        TNS__User_auth user_auth;
        user_auth.setUser_name(QString::fromLatin1("user"));
        user_auth.setPassword(QString::fromLatin1("pass"));
        qRegisterMetaType<TNS__Set_entry_result>("TNS__Set_entry_result");
        LoginJob *job = new LoginJob(&sugar);
        job->setUser_auth(user_auth);
        job->setApplication_name(QString::fromLatin1("application"));
        QEventLoop loop;
        connect(job, &LoginJob::finished, &loop, &QEventLoop::quit);
        job->start();
        loop.exec();
        const TNS__Set_entry_result result = job->return_();
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

    static QByteArray arrayResponse()
    {
        return QByteArray(xmlEnvBegin11())
            + " xmlns:tns=\"http://www.sugarcrm.com/sugarcrm\"><soap:Body>"
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
              "</soap:Body>"
            + xmlEnvEnd();
    }

private:
    static QByteArray complexTypeResponse()
    {
        return QByteArray(xmlEnvBegin11())
            + "><soap:Body xmlns:tns=\"http://www.sugarcrm.com/sugarcrm\">"
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
              "</soap:Body>"
            + xmlEnvEnd();
    }
};

QTEST_MAIN(SugarTest)

#include "test_sugar_wsdl.moc"
