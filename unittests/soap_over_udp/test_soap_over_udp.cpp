/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2019-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/

#include "httpserver_p.h"
#include "KDSoapUdpClient.h"
#include "KDSoapUdpClient_p.h"
#include "KDSoapMessage.h"
#include <QNetworkDatagram>
#include <QTest>
#include <QSignalSpy>
#include <QUuid>
#include "wsdl_wsdd-discovery-200901.h"

Q_DECLARE_METATYPE(QHostAddress)

class SoapOverUdpTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        qRegisterMetaType<KDSoapMessage>("KDSoapMessage");
        qRegisterMetaType<KDSoapHeaders>("KDSoapHeaders");
        qRegisterMetaType<QHostAddress>("QHostAddress");
    }

    void testExample()
    {
        // This is the example from the KDSoapUdpClient documentation. It is
        //   here for compile testing.

        auto soapUdpClient = new KDSoapUdpClient(this);
        connect(soapUdpClient, &KDSoapUdpClient::receivedMessage, [=](const KDSoapMessage &message, const KDSoapHeaders &headers, const QHostAddress &address, quint16 port) { Q_UNUSED(headers); Q_UNUSED(port); if (message.messageAddressingProperties().action() == QStringLiteral("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches")) { TNS__ProbeMatchesType probeMatches; probeMatches.deserialize(message); qDebug() << "Received probe match from" << address; } });
        soapUdpClient->bind(3702);

        TNS__ProbeType probe;

        KDSoapMessage message;
        message = probe.serialize(QStringLiteral("Probe"));
        message.setUse(KDSoapMessage::LiteralUse);
        message.setNamespaceUri(QStringLiteral("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01"));

        KDSoapMessageAddressingProperties addressing;
        addressing.setAction(QStringLiteral("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        addressing.setMessageID(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString(QUuid::WithoutBraces));
#else
        addressing.setMessageID(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString());
#endif
        addressing.setDestination(QStringLiteral("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01"));
        addressing.setReplyEndpointAddress(
            KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous));
        message.setMessageAddressingProperties(addressing);

        soapUdpClient->sendMessage(message, KDSoapHeaders(), QHostAddress("239.255.255.250"), 3702);
    }

    void testSendMessage()
    {
        QUdpSocket testSocket;
        bool rc = testSocket.bind();
        QVERIFY(rc);

        KDSoapUdpClient udpClient;

        udpClient.sendMessage(exampleMessage(), KDSoapHeaders(), QHostAddress::LocalHost, testSocket.localPort());

        QTRY_VERIFY(testSocket.hasPendingDatagrams());
        auto datagram = testSocket.receiveDatagram();

        QVERIFY(KDSoapUnitTestHelpers::xmlBufferCompare(datagram.data(), exampleTextData()));
    }

    void testReceiveMessage()
    {
        QUdpSocket testSocket;

        KDSoapUdpClient udpClient;
        bool rc = udpClient.bind(14951);
        QVERIFY(rc);

        QSignalSpy spy(&udpClient, &KDSoapUdpClient::receivedMessage);

        auto data = exampleTextData();
        qint64 size = testSocket.writeDatagram(data, QHostAddress::LocalHost, 14951);
        QCOMPARE(size, data.size());

        QVERIFY(spy.wait(1000));
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(KDSoapUnitTestHelpers::xmlBufferCompare(arguments.at(0).value<KDSoapMessage>().toXml(), exampleMessage().toXml()));
        QCOMPARE(arguments.at(1).value<KDSoapHeaders>().size(), 0);
        QCOMPARE(arguments.at(2).value<QHostAddress>(), QHostAddress(QHostAddress::LocalHost));
    }

private:
    QByteArray exampleTextData()
    {
        return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                          "<soap:Envelope"
                          "  xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\""
                          "  xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                          "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                          "  xmlns:wsa=\"http://www.w3.org/2005/08/addressing\""
                          "  xmlns:soap-enc=\"http://www.w3.org/2003/05/soap-encoding\""
                          "  xmlns:n1=\"http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01\">"
                          "  <soap:Header>"
                          "    <wsa:To>urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01</wsa:To>"
                          "    <wsa:Action>http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe</wsa:Action>"
                          "    <wsa:MessageID>urn:uuid:0a6dc791-2be6-4991-9af1-454778a1917a</wsa:MessageID>"
                          "  </soap:Header>"
                          "  <soap:Body>"
                          "    <n1:Probe>"
                          "      <n1:Types xmlns:i=\"http://printer.example.org/2003/imaging\">i:PrintBasic</n1:Types>"
                          "      <n1:Scopes"
                          "        MatchBy=\"http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ldap\""
                          "        >ldap:///ou=engineering,o=examplecom,c=us</n1:Scopes>"
                          "    </n1:Probe>"
                          "  </soap:Body>"
                          "</soap:Envelope>");
    }

    KDSoapMessage exampleMessage()
    {
        TNS__ProbeType probe;

        KDQName type("i:PrintBasic");
        type.setNameSpace("http://printer.example.org/2003/imaging");
        TNS__QNameListType types;
        types.setEntries(QList<KDQName>() << type);
        probe.setTypes(types);

        TNS__UriListType scopeValues;
        scopeValues.setEntries(QStringList() << "ldap:///ou=engineering,o=examplecom,c=us");
        TNS__ScopesType scopes;
        scopes.setValue(scopeValues);
        scopes.setMatchBy("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ldap");
        probe.setScopes(scopes);

        KDSoapMessage message;
        message = probe.serialize(QStringLiteral("Probe"));
        message.setUse(KDSoapMessage::LiteralUse);
        message.setNamespaceUri(QStringLiteral("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01"));

        KDSoapMessageAddressingProperties addressing;
        addressing.setAddressingNamespace(KDSoapMessageAddressingProperties::Addressing200508);
        addressing.setAction(QStringLiteral("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe"));
        addressing.setMessageID(QStringLiteral("urn:uuid:0a6dc791-2be6-4991-9af1-454778a1917a"));
        addressing.setDestination(QStringLiteral("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01"));
        message.setMessageAddressingProperties(addressing);

        return message;
    }
};

QTEST_MAIN(SoapOverUdpTest)

#include "test_soap_over_udp.moc"
