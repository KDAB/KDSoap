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
#include "wsdl_sayhello.h"

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

static const char* xmlEnvBegin11 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<soap:Envelope"
        " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
        " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"";

static const char* xmlEnvBegin =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<soap:Envelope"
    " xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\""
    " xmlns:soap-enc=\"http://www.w3.org/2003/05/soap-encoding\""
    " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
    " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
    " soap:encodingStyle=\"http://www.w3.org/2003/05/soap-encoding\"";
static const char* xmlEnvEnd = "</soap:Envelope>";

class RPCServerTest : public QObject
{
    Q_OBJECT

private:

    static QByteArray expectedHelloRequest() // http://oreilly.com/catalog/webservess/chapter/ch06.html
    {
        return QByteArray(xmlEnvBegin11) + ">"
        "<soap:Body>"
        "<n1:sayHello xmlns:n1=\"http://www.ecerami.com/wsdl/HelloService.wsdl\">"
        /*"<n1:sayHello xmlns:n1=\"urn:examples:helloservice\">" // TODO! Add support for * namespace="urn:examples:helloservice" */
             "<firstName xsi:type=\"xsd:string\">World</firstName>"
        "</n1:sayHello>"
        "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
    static QByteArray helloResponse()
    {
        return "<?xml version='1.0' encoding='UTF-8'?>"
        "<SOAP-ENV:Envelope "
           "xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
           "xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\" "
           "xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\">"
           "<SOAP-ENV:Body>"
              "<ns1:sayHelloResponse "
                 "xmlns:ns1=\"urn:examples:helloservice\" "
                 "SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
        "<return xsi:type=\"xsd:string\">Hello, World!</return>"
              "</ns1:sayHelloResponse>"
           "</SOAP-ENV:Body>"
        "</SOAP-ENV:Envelope>";
    }

    static QByteArray expectedListKeysRequest()
    {
        return QByteArray(xmlEnvBegin) + ">"
        "<soap:Body>"
         "<n1:listKeys xmlns:n1=\"urn:RpcExample\">"
          "<params xsi:type=\"n1:listKeysParams\">"
           "<module xsi:type=\"xsd:string\">Firefox</module>"
           "<base xsi:type=\"xsd:string\"></base>"
          "</params>"
         "</n1:listKeys>"
        "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
    static QByteArray listKeysResponse()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:RpcExample=\"urn:RpcExample\">"
         "<SOAP-ENV:Body>"
          "<RpcExample:listKeysResponse SOAP-ENV:encodingStyle=\"http://www.w3.org/2003/05/soap-encoding\">"
           "<result>"
            "<keys>test1</keys>"
            "<keys>test2</keys>"
            "<keys>test3</keys>"
           "</result>"
          "</RpcExample:listKeysResponse>"
         "</SOAP-ENV:Body>"
        "</SOAP-ENV:Envelope>";
    }

private Q_SLOTS:

    void testHello() // http://oreilly.com/catalog/webservess/chapter/ch06.html
    {
        HttpServerThread server(helloResponse(), HttpServerThread::Public);
        Hello_Service service;
        service.setEndPoint(server.endPoint());

        const QString resp = service.sayHello("World");
        QCOMPARE(resp, QString::fromLatin1("Hello, World!"));

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedHelloRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedHelloRequest().constData()));
        }
    }

    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testListKeys()
    {
        HttpServerThread server(listKeysResponse(), HttpServerThread::Public);

        RpcExample service;
        service.setEndPoint(server.endPoint());

        RPCEXAMPLE__ListKeysParams params;
        params.setModule(QString::fromLatin1("Firefox"));
        params.setBase(QString::fromLatin1(""));
        RPCEXAMPLE__ListKeysResult result = service.listKeys(params);

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedListKeysRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedListKeysRequest().constData()));
        }
        QCOMPARE(result.keys(), QStringList() << QString::fromLatin1("test1") << QString::fromLatin1("test2") << QString::fromLatin1("test3"));
    }
};

QTEST_MAIN(RPCServerTest)

#include "test_wsdl_rpc_server.moc"
