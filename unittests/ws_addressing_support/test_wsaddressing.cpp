/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "httpserver_p.h"

#include <QDebug>
#include <QObject>
#include <QTest>

#include "KDSoapClient/KDSoapMessageAddressingProperties.h"
#include "KDSoapClient/KDSoapMessageWriter_p.h"
#include "KDSoapNamespaceManager.h"
#include "wsdl_wsaddressing.h"

using namespace KDSoapUnitTestHelpers;

class WSAddressingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void shouldProvideCorrectPredefinedAddresses()
    {
        QString none = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::None);
        QCOMPARE(none, QString("http://www.w3.org/2005/08/addressing/none"));

        QString anonymous = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous);
        QCOMPARE(anonymous, QString("http://www.w3.org/2005/08/addressing/anonymous"));

        QString reply = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Reply);
        QCOMPARE(reply, QString("http://www.w3.org/2005/08/addressing/reply"));

        QString unspecified = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Unspecified);
        QCOMPARE(unspecified, QString("http://www.w3.org/2005/08/addressing/unspecified"));

        QString reply200303 = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Reply,
                                                                                           KDSoapMessageAddressingProperties::Addressing200303);
        QCOMPARE(reply200303, QString()); // Reply is not a thing in older than 2005/08

        QString anon200303 = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous,
                                                                                          KDSoapMessageAddressingProperties::Addressing200303);
        QCOMPARE(anon200303, QString("http://schemas.xmlsoap.org/ws/2003/03/addressing/role/anonymous"));

        QString unspecified200303 = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Unspecified,
                                                                                                 KDSoapMessageAddressingProperties::Addressing200303);
        QCOMPARE(unspecified200303, QString("http://schemas.xmlsoap.org/ws/2003/03/addressing/id/unspecified"));
    }

    void shouldWriteAProperSoapMessageWithRightsAddressingProperties()
    {
        // GIVEN
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), "http://www.ecerami.com/wsdl/HelloService");

        KDSoapMessage message;
        const QString action = QString::fromLatin1("sayHello");
        message.setUse(KDSoapMessage::EncodedUse);
        message.addArgument(QString::fromLatin1("msg"), QVariant::fromValue(QString("HelloContentMessage")), KDSoapNamespaceManager::xmlSchema2001(),
                            QString::fromLatin1("string"));
        message.setNamespaceUri(QString::fromLatin1("http://www.ecerami.com/wsdl/HelloService.wsdl"));

        // WHEN
        message.setMessageAddressingProperties(addressingProperties());
        KDSoapMessage reply = client.call(QLatin1String("sayHello"), message, action);

        // THEN
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedSoapMessage200508()));
    }

    void shouldWriteAProperSoapMessageWithAlternativeNamespace()
    {
        // GIVEN
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), "http://www.ecerami.com/wsdl/HelloService");

        KDSoapMessage message;
        const QString action = QString::fromLatin1("sayHello");
        message.setUse(KDSoapMessage::EncodedUse);
        message.addArgument(QString::fromLatin1("msg"), QVariant::fromValue(QString("HelloContentMessage")), KDSoapNamespaceManager::xmlSchema2001(),
                            QString::fromLatin1("string"));
        message.setNamespaceUri(QString::fromLatin1("http://www.ecerami.com/wsdl/HelloService.wsdl"));

        // WHEN
        message.setMessageAddressingProperties(addressingProperties200408());
        KDSoapMessage reply = client.call(QLatin1String("sayHello"), message, action);

        // THEN
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedSoapMessage200408()));
    }

    void shouldReadAProperSoapMessageWithRightsAddressingProperties()
    {
        // GIVEN
        HttpServerThread server(expectedSoapMessage200508(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), "http://www.ecerami.com/wsdl/HelloService");

        KDSoapMessage message;
        const QString action = QString::fromLatin1("sayHello");

        // WHEN
        KDSoapMessage reply = client.call(QLatin1String("sayHello"), message, action);

        // THEN
        QVERIFY(reply.hasMessageAddressingProperties());
        KDSoapMessageAddressingProperties map = reply.messageAddressingProperties();
        QCOMPARE(map.action(), QString("sayHello"));
        QCOMPARE(map.destination(), QString("http://www.ecerami.com/wsdl/HelloService"));
        QCOMPARE(map.sourceEndpointAddress(), QString("http://www.ecerami.com/wsdl/source"));
        QCOMPARE(map.faultEndpointAddress(), QString("http://www.ecerami.com/wsdl/fault"));
        QCOMPARE(map.messageID(), QString("uuid:e197db59-0982-4c9c-9702-4234d204f7f4"));
        QCOMPARE(map.replyEndpointAddress(), QString("http://www.w3.org/2005/08/addressing/anonymous"));
        QCOMPARE(map.relationships().at(0).uri, QString("uuid:http://www.ecerami.com/wsdl/someUniqueString"));
        QCOMPARE(map.relationships().at(0).relationshipType, QString("http://www.w3.org/2005/08/addressing/reply"));
        QCOMPARE(map.relationships().at(1).uri, QString("uuid:http://www.ecerami.com/wsdl/someUniqueStringBis"));
        QCOMPARE(map.relationships().at(1).relationshipType, QString("CustomTypeReply"));
        QCOMPARE(map.referenceParameters().at(0).name(), QString("myReferenceParameter"));
        QCOMPARE(map.referenceParameters().at(0).value().toString(), QString("ReferencParameterContent"));
        QCOMPARE(map.referenceParameters().at(1).name(), QString("myReferenceParameterWithChildren"));
        QCOMPARE(map.referenceParameters().at(1).childValues().size(), 2);
        QCOMPARE(map.metadata().at(0).name(), QString("myMetadata"));
        QCOMPARE(map.metadata().at(0).value().toString(), QString("MetadataContent"));
        QCOMPARE(map.metadata().at(1).name(), QString("myMetadataBis"));
        QCOMPARE(map.metadata().at(1).childValues().size(), 1);
        QCOMPARE(map.metadata().at(1).childValues().first().name(), QString("myMetadataBisChild"));
        QCOMPARE(map.metadata().at(1).childValues().first().value().toString(), QString("MetadataBisChildContent"));
    }

    void wsdlShouldWriteAProperSoapMessageWithRightsAddressingProperties()
    {
        // GIVEN
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        Hello_Service client;
        client.setEndPoint(server.endPoint());

        // Should the addressing properties be generated by kdwsdl2cpp somehow?
        client.clientInterface()->setMessageAddressingProperties(addressingProperties());

        // WHEN
        client.sayHello(QString("HelloContentMessage"));

        // THEN
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedSoapMessage200508()));
    }


private:
    static KDSoapMessageAddressingProperties addressingProperties()
    {
        KDSoapMessageAddressingProperties map;

        // with some message addressing properties
        map.setAction("sayHello");
        map.setDestination("http://www.ecerami.com/wsdl/HelloService");
        map.setSourceEndpointAddress("http://www.ecerami.com/wsdl/source");
        map.setFaultEndpoint(KDSoapEndpointReference("http://www.ecerami.com/wsdl/fault"));
        map.setMessageID("uuid:e197db59-0982-4c9c-9702-4234d204f7f4");
        map.setReplyEndpointAddress(KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous));

        // two relationships related to previous message
        KDSoapMessageRelationship::Relationship relationship("uuid:http://www.ecerami.com/wsdl/someUniqueString"); // no type means implicit Reply
        KDSoapMessageRelationship::Relationship relationshipBis("uuid:http://www.ecerami.com/wsdl/someUniqueStringBis", "CustomTypeReply");
        map.addRelationship(relationship);
        map.addRelationship(relationshipBis);

        // some reference parameters...

        // one with a value
        KDSoapValue refParam("myReferenceParameter", "ReferencParameterContent");
        map.addReferenceParameter(refParam);

        // an other one, with children
        KDSoapValue childOne("myReferenceParameterChildOne", "ChildOneContent");
        KDSoapValue childTwo("myReferenceParameterChildTwo", "ChildTwoContent");
        KDSoapValueList childrenList;
        childrenList << childOne << childTwo;
        KDSoapValue refParamWithChildren("myReferenceParameterWithChildren", childrenList);
        map.addReferenceParameter(refParamWithChildren);

        // some metadata
        KDSoapValueList metadataContainer;
        KDSoapValue metadata("myMetadata", "MetadataContent");
        metadataContainer << metadata;

        KDSoapValue child("myMetadataBisChild", "MetadataBisChildContent");
        KDSoapValueList childList;
        childList << child;
        KDSoapValue metadataBis("myMetadataBis", childList);

        map.setMetadata(metadataContainer);
        map.addMetadata(metadataBis);

        return map;
    }

    static KDSoapMessageAddressingProperties addressingProperties200408()
    {
        KDSoapMessageAddressingProperties map = addressingProperties();
        map.setAddressingNamespace(KDSoapMessageAddressingProperties::Addressing200408);
        return map;
    }

    static QByteArray expectedSoapMessage200408()
    {
        return QByteArray(xmlEnvBegin11()) + " xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\"" + expectedSoapMessagePartial();
    }

    static QByteArray expectedSoapMessage200508()
    {
        return QByteArray(xmlEnvBegin11()) + " xmlns:wsa=\"http://www.w3.org/2005/08/addressing\"" + expectedSoapMessagePartial();
    }

    static QByteArray expectedSoapMessagePartial()
    {
        return QByteArray(" xmlns:n1=\"http://www.ecerami.com/wsdl/HelloService.wsdl\">")
            + "<soap:Header>"
              "<wsa:To>http://www.ecerami.com/wsdl/HelloService</wsa:To>"
              "<wsa:From>"
              "<wsa:Address>http://www.ecerami.com/wsdl/source</wsa:Address>"
              "</wsa:From>"
              "<wsa:ReplyTo>"
              "<wsa:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa:Address>"
              "</wsa:ReplyTo>"
              "<wsa:FaultTo>"
              "<wsa:Address>http://www.ecerami.com/wsdl/fault</wsa:Address>"
              "</wsa:FaultTo>"
              "<wsa:Action>sayHello</wsa:Action>"
              "<wsa:MessageID>uuid:e197db59-0982-4c9c-9702-4234d204f7f4</wsa:MessageID>"
              "<wsa:RelatesTo>uuid:http://www.ecerami.com/wsdl/someUniqueString</wsa:RelatesTo>"
              "<wsa:RelatesTo RelationshipType=\"CustomTypeReply\">uuid:http://www.ecerami.com/wsdl/someUniqueStringBis</wsa:RelatesTo>"
              "<wsa:ReferenceParameters>"
              "<wsa:myReferenceParameter>ReferencParameterContent</wsa:myReferenceParameter>"
              "<wsa:myReferenceParameterWithChildren>"
              "<wsa:myReferenceParameterChildOne>ChildOneContent</wsa:myReferenceParameterChildOne>"
              "<wsa:myReferenceParameterChildTwo>ChildTwoContent</wsa:myReferenceParameterChildTwo>"
              "</wsa:myReferenceParameterWithChildren>"
              "</wsa:ReferenceParameters>"
              "<wsa:Metadata>"
              "<wsa:myMetadata>MetadataContent</wsa:myMetadata>"
              "<wsa:myMetadataBis>"
              "<wsa:myMetadataBisChild>MetadataBisChildContent</wsa:myMetadataBisChild>"
              "</wsa:myMetadataBis>"
              "</wsa:Metadata>"
              "</soap:Header>"
              "<soap:Body>"
              "<n1:sayHello><msg xsi:type=\"xsd:string\">HelloContentMessage</msg></n1:sayHello>"
              "</soap:Body>"
            + xmlEnvEnd();
    }

    static QByteArray emptyResponse()
    {
        return QByteArray(xmlEnvBegin11()) + "><soap:Body/>";
    }
};

QTEST_MAIN(WSAddressingTest)

#include "test_wsaddressing.moc"
