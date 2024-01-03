/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDDateTime.h"
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "httpserver_p.h"
#include "wsdl_DWService.h"
#include <QDebug>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

class DWServiceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testLogin()
    {
        HttpServerThread server(loginResponse(), HttpServerThread::Public);
        KDAB::DWService service;
        service.setEndPoint(server.endPoint());

        KDAB::TNS__Login loginParams;
        loginParams.setUserName(QString::fromLatin1("Foo"));
        loginParams.setOrganization(QString::fromLatin1("KDAB"));
        const KDAB::TNS__LoginResponse resp = service.login(loginParams);

        // Check what we sent
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<n1:Login xmlns:n1=\"http://tempuri.org/\">"
              "<n1:userName>Foo</n1:userName>"
              // minoccurs=0, so we don't need this: "<n1:password xsi:nil=\"true\"></n1:password>"
              "<n1:organization>KDAB</n1:organization>"
              "</n1:Login>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        const KDAB::Q1__ClientServiceSession session = resp.loginResult();
        const KDAB::SER__Guid sessionId = session.sessionID();
        QCOMPARE(sessionId.value(), QString::fromLatin1("65a65c1f-2613-47d0-89ec-1c7b1fe34777"));
    }

    void testLogoff()
    {
        HttpServerThread server(loginResponse(), HttpServerThread::Public);
        KDAB::DWService service;
        service.setEndPoint(server.endPoint());

        KDAB::Q1__ClientServiceSession session;
        session.setSessionID(KDAB::SER__Guid(QString::fromLatin1("65a65c1f-2613-47d0-89ec-1c7b1fe34777")));
        KDAB::TNS__Logoff logoffParams;
        logoffParams.setClientSession(session);
        service.logoff(logoffParams);

        // Check what we sent
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<n1:Logoff xmlns:n1=\"http://tempuri.org/\">"
              "<n1:clientSession><n2:SessionID "
              "xmlns:n2=\"http://schemas.datacontract.org/2004/07/"
              "DocuWare.WebServices.GAPIFunctionality.DataContracts\">65a65c1f-2613-47d0-89ec-1c7b1fe34777</n2:SessionID></n1:clientSession>"
              "</n1:Logoff>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

private:
    static QByteArray loginResponse()
    {
        return QByteArray(xmlEnvBegin11())
            + " xmlns:dw=\"http://schemas.novell.com/2005/01/GroupWise/groupwise.wsdl\"><soap:Body>"
              "<dw:LoginResponse>"
              "<dw:LoginResult><dw:SessionID>65a65c1f-2613-47d0-89ec-1c7b1fe34777</dw:SessionID></dw:LoginResult>"
              "</dw:LoginResponse>"
              "</soap:Body>"
            + xmlEnvEnd();
    }
};

QTEST_MAIN(DWServiceTest)

#include "test_dwservice_wsdl.moc"
