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

#include "wsdl_mywsdl_rpc.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

class WsdlRPCTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testRequest()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
                                  "<kdab:addEmployeeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\"><kdab:bStrReturn>Foo</kdab:bStrReturn></kdab:addEmployeeResponse>"
                                  " </soap:Body>" + xmlEnvEnd();
        HttpServerThread server(responseData, HttpServerThread::Public);

        // For testing the http server with telnet or wget:
        //httpGet(server.endPoint());
        //QEventLoop testLoop;
        //testLoop.exec();

        MyWsdl service;
        service.setEndPoint(server.endPoint());
        KDAB__EmployeeAchievements achievements;
        QList<KDAB__EmployeeAchievement> lst;
        KDAB__EmployeeAchievement achievement1;
        achievement1.setType(QString::fromLatin1("Project"));
        achievement1.setLabel(QString::fromLatin1("Management"));
        lst.append(achievement1);
        KDAB__EmployeeAchievement achievement2;
        achievement2.setType(QString::fromLatin1("Development"));
        achievement2.setLabel(QString::fromLatin1("C++"));
        lst.append(achievement2);
        achievements.setItems(lst);
        KDAB__EmployeeType employeeType;
        employeeType.setType(KDAB__EmployeeTypeEnum::Developer);
        employeeType.setOtherRoles(QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader);
        KDAB__EmployeeTypeEnumList otherRoles;
        otherRoles.setEntries(QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader << KDAB__EmployeeTypeEnum::Developer);
        employeeType.setOtherRolesAsList(otherRoles);
        KDAB__LottoNumbers lottoNumbers;
        lottoNumbers.setEntries(QList<int>() << 7 << 21 << 30 << 42);
        employeeType.setLottoNumbers(lottoNumbers);
        employeeType.setTeam(QList<KDAB__TeamName>() << QString::fromLatin1("Minitel"));
        KDAB__AnonListType anonList;
        anonList.setEntries(QList<KDAB__AnonListTypeListItem>() << KDAB__AnonListTypeListItem::Detailed << KDAB__AnonListTypeListItem::DetailedMerged);
        employeeType.setAnonList(anonList);
        KDAB__JeansSize jeansSize;
        jeansSize.setValue(24);

        KDAB__LoginElement login;
        login.setUser(QLatin1String("foo"));
        login.setPass(QLatin1String("bar"));
        KDAB__SessionElement session;
        session.setSessionId(QLatin1String("id"));

        service.setLoginHeader(login);
        service.setSessionHeader(session);

        QString ret = service.addEmployee(employeeType,
                                          QString::fromLatin1("David Faure"),
                                          QString::fromLatin1("France"),
                                          achievements,
                                          jeansSize);
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QString::fromLatin1("Foo"));
        const QByteArray expectedHeader =
                "<soap:Header>"
                "<n1:LoginHeader xsi:type=\"n1:LoginElement\">"
                "<user xsi:type=\"xsd:string\">foo</user>"
                "<pass xsi:type=\"xsd:string\">bar</pass>"
                "</n1:LoginHeader>"
                "<n1:SessionHeader xsi:type=\"n1:SessionElement\">"
                "<sessionId xsi:type=\"xsd:string\">id</sessionId>"
                "</n1:SessionHeader>"
                "</soap:Header>";
        // Check what we sent
        QByteArray requestXmlTemplate =
            QByteArray(xmlEnvBegin11()) +
            " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">%1"
            "<soap:Body>"
            "<n1:addEmployee>"
            + serializedEmployee() +
            "</n1:addEmployee>"
            "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        {
            QByteArray expectedRequestXml = requestXmlTemplate;
            expectedRequestXml.replace("%1", expectedHeader);
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedRequestXml.constData()));
            QVERIFY(server.receivedHeaders().contains("SoapAction: \"http://www.kdab.com/AddEmployee\""));
        }

        // Test utf8
        {
            // This second call also tests that persistent headers are indeed persistent.
            server.resetReceivedBuffers();
            requestXmlTemplate.replace("David Faure", "Hervé");
            requestXmlTemplate.replace("France", "фгн7");
            QByteArray expectedRequestXml = requestXmlTemplate;
            expectedRequestXml.replace("%1", expectedHeader);
            ret = service.addEmployee(employeeType,
                                      QString::fromUtf8("Hervé"),
                                      QString::fromUtf8("фгн7"), // random russian letters
                                      achievements, jeansSize);
            QVERIFY(service.lastError().isEmpty());
            QCOMPARE(ret, QString::fromLatin1("Foo"));
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        }

        // Test removing headers
        {
            server.resetReceivedBuffers();
            service.clearLoginHeader();
            service.clearSessionHeader();
            ret = service.addEmployee(employeeType,
                                      QString::fromUtf8("Hervé"),
                                      QString::fromUtf8("фгн7"), // random russian letters
                                      achievements, jeansSize);
            QByteArray expectedRequestXml = requestXmlTemplate;
            expectedRequestXml.replace("%1", "<soap:Header/>");
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        }
    }

    void testResponse()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
                                  "<n1:getEmployeeResponse xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
                                  "<n1:employee>"
                                   + serializedEmployee() +
                                  "</n1:employee>"
                                  "</n1:getEmployeeResponse>"
        " </soap:Body>" + xmlEnvEnd();
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdl service;
        service.setEndPoint(server.endPoint());
        const KDAB__Employee employee = service.getEmployee(QString::fromLatin1("David Faure"));
        const KDAB__EmployeeType employeeType = employee.employeeType();
        QCOMPARE(employeeType.team().first().value().value(), QString::fromLatin1("Minitel"));
        QCOMPARE(employeeType.type().type(), KDAB__EmployeeTypeEnum::Developer);
        QCOMPARE(employeeType.otherRoles(), QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader);
        QCOMPARE(employeeType.otherRolesAsList().entries(), QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader << KDAB__EmployeeTypeEnum::Developer);
        QCOMPARE(employeeType.lottoNumbers().entries(), QList<int>() << 7 << 21 << 30 << 42);
        QCOMPARE(employee.employeeName().value().value(), QString::fromLatin1("David Faure"));
        QCOMPARE(employee.employeeCountry().value(), QString::fromLatin1("France"));
        QCOMPARE(employee.employeeJeansSize().value().toInt(), 24);
    }

    // Test calls with 'simple type' arguments
    // Same as the call made by builtinhttp, but here using the wsdl-generated code
    void testSimpleType()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::Public);
        MyWsdl service;
        service.setEndPoint(server.endPoint());

        KDAB__LimitedString employeeCountry = service.getEmployeeCountry(KDAB__EmployeeName(QString::fromUtf8("David Ä Faure")));
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(employeeCountry.value(), QString::fromLatin1("France"));
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
        QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedCountryRequest().constData()));
    }

    // Test enum deserialization
    void testEnums()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
                                  "<kdab:getEmployeeTypeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">"
                                    "<kdab:employeeType kdab:type=\"Developer\">"
                                      "<kdab:otherRoles>TeamLeader</kdab:otherRoles>"
                                      "<kdab:team>Minitel</kdab:team>"
                                    "</kdab:employeeType>"
                                  "</kdab:getEmployeeTypeResponse>"
                                  "</soap:Body>" + xmlEnvEnd();
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdl service;
        service.setEndPoint(server.endPoint());

        KDAB__EmployeeType employeeType = service.getEmployeeType(KDAB__EmployeeName(QLatin1String("Joe")));
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(employeeType.team().first().value().value(), QLatin1String("Minitel"));
        QCOMPARE(employeeType.otherRoles().count(), 1);
        QCOMPARE(employeeType.otherRoles().at(0).type(), KDAB__EmployeeTypeEnum::TeamLeader);
        QCOMPARE((int)employeeType.type().type(), (int)KDAB__EmployeeTypeEnum::Developer);
    }

    void testByteArrays()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin11()) + "><soap:Body>"
                                  "<kdab:sendTelegramResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">"
                                    "<kdab:telegram>466f6f</kdab:telegram>"
                                  "</kdab:sendTelegramResponse>"
                                  "</soap:Body>" + xmlEnvEnd();
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdl service;
        service.setEndPoint(server.endPoint());

        const KDAB__Telegram ret = service.sendTelegram(KDAB__Telegram("Hello"));
        QCOMPARE(service.lastError(), QString());
        QCOMPARE(ret.value(), QByteArray("Foo"));

        const QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin11()) + ">"
            "<soap:Body>"
            "<n1:sendTelegram xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
               "<telegram>48656c6c6f</telegram>"
            "</n1:sendTelegram>"
            "</soap:Body>" + xmlEnvEnd();
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

private:
    static QByteArray serializedEmployeeType() {
        return QByteArray(
                "<employeeType xsi:type=\"n1:EmployeeType\" type=\"Developer\">"
                "<otherRoles xsi:type=\"n1:EmployeeTypeEnum\">TeamLeader</otherRoles>"
                "<otherRolesAsList xsi:type=\"n1:EmployeeTypeEnumList\">TeamLeader Developer</otherRolesAsList>"
                "<lottoNumbers xsi:type=\"n1:LottoNumbers\">7 21 30 42</lottoNumbers>"
                "<team xsi:type=\"n1:TeamName\">Minitel</team>"
                "<anonList xsi:type=\"n1:AnonListType\">Detailed DetailedMerged</anonList>"
                "</employeeType>");
    }
    static QByteArray serializedEmployee() {
        return serializedEmployeeType() +
                "<employeeName xsi:type=\"n1:EmployeeName\">David Faure</employeeName>"
                "<employeeCountry xsi:type=\"n1:LimitedString\">France</employeeCountry>"
                "<employeeAchievements xsi:type=\"n1:EmployeeAchievements\" soap-enc:arrayType=\"n1:EmployeeAchievement[2]\">"
                "<item xsi:type=\"n1:EmployeeAchievement\">"
                "<type xsi:type=\"xsd:string\">Project</type>"
                "<label xsi:type=\"xsd:string\">Management</label>"
                "</item>"
                "<item xsi:type=\"n1:EmployeeAchievement\">"
                "<type xsi:type=\"xsd:string\">Development</type>"
                "<label xsi:type=\"xsd:string\">C++</label>"
                "</item>"
                "</employeeAchievements>"
                "<employeeJeansSize xsi:type=\"n1:JeansSize\">24</employeeJeansSize>";
    }

    static QByteArray countryResponse() {
        return QByteArray(xmlEnvBegin11()) + "><soap:Body>"
                "<kdab:getEmployeeCountryResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">"
                "<kdab:employeeCountry>France</kdab:employeeCountry>"
                "</kdab:getEmployeeCountryResponse>"
                " </soap:Body>" + xmlEnvEnd();
    }
    static QByteArray expectedCountryRequest() {
        return QByteArray(xmlEnvBegin11()) +
                "><soap:Body>"
                "<n1:getEmployeeCountry xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
                "<employeeName>"
                "David Ä Faure"
                "</employeeName>"
                "</n1:getEmployeeCountry>"
                "</soap:Body>" + xmlEnvEnd()
                + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
};

QTEST_MAIN(WsdlRPCTest)

#include "test_wsdl_rpc.moc"
