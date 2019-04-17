/****************************************************************************
** Copyright (C) 2017-2019 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
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

#include "wsdl_calc.h"
#include "httpserver_p.h"
#include <QTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapServer.h>
#include <KDSoapNamespaceManager.h>

using namespace KDSoapUnitTestHelpers;


class CalcTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAddRequestRemote()
    {
        Calc service;

        double result = service.add(5, 5);
        QCOMPARE(service.lastError(), QString());
        QCOMPARE(result, 10.0);
    }

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
    static QByteArray addResponseXml() {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
               "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" "
                "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
                "xmlns:ns=\"urn:calc\"><SOAP-ENV:Body SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
                "<ns:addResponse>"
                "<result>10</result>"
                "</ns:addResponse>"
               "</SOAP-ENV:Body>"
               "</SOAP-ENV:Envelope>";
    }
    static QByteArray expectedAddRequestXml() {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
               "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
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
