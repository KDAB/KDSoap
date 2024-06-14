/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2017-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "httpserver_p.h"
#include "wsdl_calc.h"
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapNamespaceManager.h>
#include <KDSoapServer.h>
#include <QDebug>
#include <QEventLoop>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

class CalcTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
#if 0 // http://websrv.cs.fsu.edu/~engelen/calcserver.cgi no longer exists
    void testAddRequestRemote()
    {
        Calc service;

        double result = service.add(5, 5);
        QCOMPARE(service.lastError(), QString());
        QCOMPARE(result, 10.0);
    }
#endif

    void testAddRequestLocal()
    {
        Calc service;
        HttpServerThread server(addResponseXml(), HttpServerThread::Public);
        service.setEndPoint(server.endPoint());

        double result = service.add(5, 5);
        QCOMPARE(service.lastError(), QString());
        QCOMPARE(result, 10.0);

        // Check what we sent
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedAddRequestXml()));
    }

private:
    static QByteArray addResponseXml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
               "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
               "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" "
               "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
               "xmlns:ns=\"urn:calc\"><SOAP-ENV:Body SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
               "<ns:addResponse>"
               "<result>10</result>"
               "</ns:addResponse>"
               "</SOAP-ENV:Body>"
               "</SOAP-ENV:Envelope>";
    }
    static QByteArray expectedAddRequestXml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
               "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" "
               "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
               "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
               "<soap:Body>"
               "<n1:add xmlns:n1=\"urn:calc\">"
               "<a xsi:type=\"xsd:double\">5</a><b xsi:type=\"xsd:double\">5</b>"
               "</n1:add>"
               "</soap:Body>"
               "</soap:Envelope>";
    }
};

QTEST_MAIN(CalcTest)

#include "test_calc.moc"
