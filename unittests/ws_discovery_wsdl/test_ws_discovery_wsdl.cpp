/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "wsdl_ws_discovery200504.h"
#include "httpserver_p.h"
#include "KDSoapMessageReader_p.h"
#include <QTest>

// Apperently the original file is not available anymore: http://schemas.xmlsoap.org/ws/2005/04/discovery/ws-discovery.wsdl
// However it is available via: https://web.archive.org/web/20070225062601/http://schemas.xmlsoap.org/ws/2005/04/discovery/ws-discovery.wsdl

using namespace KDSoapUnitTestHelpers;

class WSDiscoveryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testGeneratingProbe()
    {
        // Types
        KDQName type(QString("i:PrintBasic"));
        type.setNameSpace(QString("http://printer.example.org/2003/imaging"));
        TNS__QNameListType types;
        types.setEntries(QList<KDQName>() << type);

        // Scopes
        QStringList uriStringList;
        uriStringList << "ldap:///ou=engineering,o=examplecom,c=us";
        TNS__UriListType uriList;
        uriList.setEntries(uriStringList);
        TNS__ScopesType scopes;
        scopes.setValue(uriList);
        scopes.setMatchBy("http://schemas.xmlsoap.org/ws/2005/04/discovery/ldap");

        // Probe
        TNS__ProbeType probe;
        probe.setTypes(types);
        probe.setScopes(scopes);

        // Check what we sent
        QByteArray expectedRequestXml = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                                   "<n1:Probe"
                                                   " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
                                                   " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                                                   " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                                                   " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
                                                   " xmlns:n1=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"
                                                   "<n1:Types xmlns:i=\"http://printer.example.org/2003/imaging\">i:PrintBasic</n1:Types>"
                                                   "<n1:Scopes MatchBy=\"http://schemas.xmlsoap.org/ws/2005/04/discovery/ldap\">"
                                                   "ldap:///ou=engineering,o=examplecom,c=us"
                                                   "</n1:Scopes>"
                                                   "</n1:Probe>");

        KDSoapValue value = probe.serialize("Probe");
        value.setNamespaceUri("http://schemas.xmlsoap.org/ws/2005/04/discovery");
        QVERIFY(xmlBufferCompare(value.toXml(), expectedRequestXml));
    }

    void testReceivingProbe()
    {
        QByteArray incommingXmlData = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                                 "<s:Envelope"
                                                 " xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\""
                                                 " xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\""
                                                 " xmlns:i=\"http://printer.example.org/2003/imaging\""
                                                 " xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" >"
                                                 "<s:Body>"
                                                 "<d:ProbeMatches>"
                                                 "<d:ProbeMatch>"
                                                 "<a:EndpointReference>"
                                                 "<a:Address>"
                                                 "uuid:98190dc2-0890-4ef8-ac9a-5940995e6119"
                                                 "</a:Address>"
                                                 "</a:EndpointReference>"
                                                 "<d:Types>i:PrintBasic i:PrintAdvanced</d:Types>"
                                                 "<d:Scopes>"
                                                 "ldap:///ou=engineering,o=examplecom,c=us "
                                                 "ldap:///ou=floor1,ou=b42,ou=anytown,o=examplecom,c=us "
                                                 "http://itdept/imaging/deployment/2004-12-04"
                                                 "</d:Scopes>"
                                                 "<d:XAddrs>http://prn-example/PRN42/b42-1668-a</d:XAddrs>"
                                                 "<d:MetadataVersion>75965</d:MetadataVersion>"
                                                 "</d:ProbeMatch>"
                                                 "</d:ProbeMatches>"
                                                 "</s:Body>"
                                                 "</s:Envelope>");

        KDSoapMessage replyMessage;
        KDSoapHeaders replyHeaders;

        KDSoapMessageReader reader;
        reader.xmlToMessage(incommingXmlData, &replyMessage, 0, &replyHeaders, KDSoap::SOAP1_2);

        TNS__ProbeMatchesType probeMatches;
        probeMatches.deserialize(replyMessage);

        const QList<TNS__ProbeMatchType> &probeMatchList = probeMatches.probeMatch();
        QCOMPARE(probeMatchList.size(), 1);
        const TNS__ProbeMatchType &probeMatch = probeMatchList.value(0);
        QCOMPARE(QString(probeMatch.endpointReference().address()), QString("uuid:98190dc2-0890-4ef8-ac9a-5940995e6119"));
        const QList<KDQName> &typeList = probeMatch.types().entries();
        QCOMPARE(typeList.size(), 2);
        QCOMPARE(typeList.value(0).localName(), QString("PrintBasic"));
        QCOMPARE(typeList.value(0).prefix(), QString("i"));
        QCOMPARE(typeList.value(0).nameSpace(), QString("http://printer.example.org/2003/imaging"));
        QCOMPARE(typeList.value(1).localName(), QString("PrintAdvanced"));
        QCOMPARE(typeList.value(1).prefix(), QString("i"));
        QCOMPARE(typeList.value(1).nameSpace(), QString("http://printer.example.org/2003/imaging"));
        const QStringList &scopeList = probeMatch.scopes().value().entries();
        QCOMPARE(scopeList.size(), 3);
        QCOMPARE(scopeList.value(0), QString("ldap:///ou=engineering,o=examplecom,c=us"));
        QCOMPARE(scopeList.value(1), QString("ldap:///ou=floor1,ou=b42,ou=anytown,o=examplecom,c=us"));
        QCOMPARE(scopeList.value(2), QString("http://itdept/imaging/deployment/2004-12-04"));
        const QStringList &xaddrList = probeMatch.xAddrs().entries();
        QCOMPARE(xaddrList.size(), 1);
        QCOMPARE(xaddrList.value(0), QString("http://prn-example/PRN42/b42-1668-a"));
        QCOMPARE(int(probeMatch.metadataVersion()), 75965);
    }
};

QTEST_MAIN(WSDiscoveryTest)

#include "test_ws_discovery_wsdl.moc"
