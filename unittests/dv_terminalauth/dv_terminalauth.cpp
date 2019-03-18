/****************************************************************************
** Copyright (C) 2010-2019 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

#include "wsdl_WS_DV_TerminalAuth.h"

#include "httpserver_p.h"
#include <QTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapNamespaceManager.h>

using namespace KDSoapUnitTestHelpers;

class TestDVTerminalAuth : public QObject
{
    Q_OBJECT

private:

    static QByteArray expectedGetCACertRequest()
    {
        return QByteArray(xmlEnvBegin11()) + ">"
               "<soap:Body>"
               "<n1:GetCACertificates xmlns:n1=\"uri:EAC-PKI-DV-Protocol/1.1\">"
               "<callbackIndicator>callback_not_possible</callbackIndicator>"
                "<n1:messageID/>"
                "<n1:responseURL/>"
               "</n1:GetCACertificates>"
               "</soap:Body>" + xmlEnvEnd()
               + '\n'; // added by QXmlStreamWriter::writeEndDocument;
    }

    static QByteArray getCACertResponse()
    {
        return "<?xml version='1.0' encoding='UTF-8'?>"
"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body><ns1:GetCACertificatesResponse xmlns:ns1=\"uri:EAC-PKI-DV-Protocol/1.1\"><Result xmlns:ns2=\"uri:eacBT/1.1\"><ns2:returnCode>ok_cert_available</ns2:returnCode><ns2:certificateSeq><ns2:certificate>" + QByteArray("XXXYYYZZZ").toBase64() + "</ns2:certificate></ns2:certificateSeq></Result></ns1:GetCACertificatesResponse></soap:Body></soap:Envelope>";
    }

private Q_SLOTS:

    void testGetCACertificates()
    {
        HttpServerThread server(getCACertResponse(), HttpServerThread::Public);
        EAC_DV_ProtocolService service;
        service.setEndPoint(server.endPoint());

        NS__CallbackIndicatorType indicator;
        indicator.setType(NS__CallbackIndicatorType::Callback_not_possible);
        NS__OptionalMessageIDType messageId;
        NS__OptionalStringType responseURL;
        NS__GetCACertificatesResult result = service.getCACertificates(indicator, messageId, responseURL);

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedGetCACertRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedGetCACertRequest().constData()));
            QCOMPARE(server.header("SoapAction").constData(), "\"\""); // SOAP-135
        }

        QCOMPARE(NS__GetCACertificates_returnCodeType::Type(result.returnCode()), NS__GetCACertificates_returnCodeType::Ok_cert_available);
        QCOMPARE(result.certificateSeq().certificate(), QList<QByteArray>() << QByteArray("XXXYYYZZZ"));
    }
};

QTEST_MAIN(TestDVTerminalAuth)

#include "dv_terminalauth.moc"
