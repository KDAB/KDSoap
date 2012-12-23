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

#include "wsdl_rpcexample.h"

#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapPendingCallWatcher.h>
#include <KDSoapNamespaceManager.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace KDSoapUnitTestHelpers;

static const char* xmlEnvBegin =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<soap:Envelope "
    " xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\""
    " xmlns:soap-enc=\"http://www.w3.org/2003/05/soap-encoding\""
    " xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\""
    " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\""
    " soap:encodingStyle=\"http://www.w3.org/2003/05/soap-encoding\"";
static const char* xmlEnvEnd = "</soap:Envelope>";

class RPCServerTest : public QObject
{
    Q_OBJECT

private:
    static QByteArray expectedRequest()
    {
        return QByteArray(xmlEnvBegin) + ">"
        "<soap:Body>"
         "<n1:listKeys xmlns:n1=\"urn:RpcExample\">"
           "<params>"
             "<module>System Info</module>"
             "<base xmlns:n2=\"http://www.w3.org/2001/XMLSchema-instance\" n2:nil=\"true\"/>"
           "</params>"
         "</n1:listKeys>"
        "</soap:Body>";
    }
    static QByteArray listKeysResponse()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\" xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\" soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
         "<soap:Body>"
          "<n1:result xmlns:n1=\"urn:RpcExample\">"
           "<item>testKey</item>"
           "<item>testKey2</item>"
           "<item>testKey3</item>"
          "</n1:result>"
         "</soap:Body>"
        "</soap:Envelope>";
    }

private Q_SLOTS:

    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testListKeys()
    {
        HttpServerThread server(listKeysResponse(), HttpServerThread::Public);

        RpcExample service;
        service.setEndPoint(server.endPoint());

        RPCEXAMPLE__ListKeysParams params;
        params.setModule(QString::fromLatin1("System Info"));
        RPCEXAMPLE__ListKeysResult result = service.listKeys(params);
        QCOMPARE(result.keys(), QStringList() << QString::fromLatin1("testKey") << QString::fromLatin1("testKey2"));

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedRequest().constData()));
            QVERIFY(server.receivedHeaders().contains("SoapAction: \"http://www.kdab.com/AddEmployee\""));
        }
    }
};

QTEST_MAIN(RPCServerTest)

#include "test_wsdl_rpc_server.moc"
