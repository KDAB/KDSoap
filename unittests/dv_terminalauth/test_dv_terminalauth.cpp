/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "wsdl_WS_DV_TerminalAuth.h"

#include "httpserver_p.h"
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapNamespaceManager.h>
#include <QDebug>
#include <QEventLoop>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

class TestDVTerminalAuth : public QObject
{
    Q_OBJECT

private:
    static QByteArray expectedGetCACertRequest()
    {
        return QByteArray(xmlEnvBegin11())
            + ">"
              "<soap:Body>"
              "<n1:GetCACertificates xmlns:n1=\"uri:EAC-PKI-DV-Protocol/1.1\">"
              "<callbackIndicator>callback_not_possible</callbackIndicator>"
              "<n1:messageID/>"
              "<n1:responseURL/>"
              "</n1:GetCACertificates>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument;
    }

    static QByteArray getCACertResponse()
    {
        return "<?xml version='1.0' encoding='UTF-8'?>"
               "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body><ns1:GetCACertificatesResponse "
               "xmlns:ns1=\"uri:EAC-PKI-DV-Protocol/1.1\"><Result "
               "xmlns:ns2=\"uri:eacBT/1.1\"><ns2:returnCode>ok_cert_available</ns2:returnCode><ns2:certificateSeq><ns2:certificate>"
            + QByteArray("XXXYYYZZZ").toBase64()
            + "</ns2:certificate></ns2:certificateSeq></Result></ns1:GetCACertificatesResponse></soap:Body></soap:Envelope>";
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

#include "test_dv_terminalauth.moc"
