/****************************************************************************
** Copyright (C) 2010-2018 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

#include "httpserver_p.h"

#include <QtTest/QtTest>
#include <QDebug>
#include <QObject>

#include "KDSoapClient/KDSoapMessageWriter_p.h"
#include "KDSoapClient/KDSoapMessageAddressingProperties.h"
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
    }

    void shouldWriteAProperSoapMessageWithRightsAddressingProperties()
    {
        // GIVEN
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), "http://www.ecerami.com/wsdl/HelloService");

        KDSoapMessage message;
        const QString action = QString::fromLatin1("sayHello");
        message.setUse(KDSoapMessage::EncodedUse);
        message.addArgument(QString::fromLatin1("msg"), QVariant::fromValue(QString("HelloContentMessage")), KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("string"));
        message.setNamespaceUri(QString::fromLatin1("http://www.ecerami.com/wsdl/HelloService.wsdl"));

        // WHEN
        message.setMessageAddressingProperties(addressingProperties());
        KDSoapMessage reply = client.call(QLatin1String("sayHello"), message, action);

        // THEN
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedSoapMessage()));
    }

    void shouldReadAProperSoapMessageWithRightsAddressingProperties()
    {
        // GIVEN
        HttpServerThread server(expectedSoapMessage(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), "http://www.ecerami.com/wsdl/HelloService");

        KDSoapMessage message;
        const QString action = QString::fromLatin1("sayHello");

        // WHEN
        KDSoapMessage reply = client.call(QLatin1String("sayHello"), message, action);

        // THEN
        QVERIFY(reply.hasMessageAddressingProperties());
        KDSoapMessageAddressingProperties map = reply.messageAddressingProperties();
        QCOMPARE(map.action(), "sayHello");
        QCOMPARE(map.destination(), "http://www.ecerami.com/wsdl/HelloService");
        QCOMPARE(map.sourceEndpointAddress(), "http://www.ecerami.com/wsdl/source");
        QCOMPARE(map.faultEndpointAddress(), "http://www.ecerami.com/wsdl/fault");
        QCOMPARE(map.messageID(), "uuid:e197db59-0982-4c9c-9702-4234d204f7f4");
        QCOMPARE(map.replyEndpointAddress(), "http://www.w3.org/2005/08/addressing/anonymous");
        QCOMPARE(map.relationships().at(0).uri, "uuid:http://www.ecerami.com/wsdl/someUniqueString");
        QCOMPARE(map.relationships().at(0).relationshipType, "http://www.w3.org/2005/08/addressing/reply");
        QCOMPARE(map.relationships().at(1).uri, "uuid:http://www.ecerami.com/wsdl/someUniqueStringBis");
        QCOMPARE(map.relationships().at(1).relationshipType, "CustomTypeReply");
        QCOMPARE(map.referenceParameters().at(0).name(), "myReferenceParameter");
        QCOMPARE(map.referenceParameters().at(0).value().toString(), "ReferencParameterContent");
        QCOMPARE(map.referenceParameters().at(1).name(), "myReferenceParameterWithChildren");
        QCOMPARE(map.referenceParameters().at(1).childValues().size(), 2);
        QCOMPARE(map.metadata().at(0).name(), "myMetadata");
        QCOMPARE(map.metadata().at(0).value().toString(), "MetadataContent");
        QCOMPARE(map.metadata().at(1).name(), "myMetadataBis");
        QCOMPARE(map.metadata().at(1).childValues().size(), 1);
        QCOMPARE(map.metadata().at(1).childValues().first().name(), "myMetadataBisChild");
        QCOMPARE(map.metadata().at(1).childValues().first().value().toString(), "MetadataBisChildContent");
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
        KDSoapValueList childList; childList << child;
        KDSoapValue metadataBis("myMetadataBis", childList);

        map.setMetadata(metadataContainer);
        map.addMetadata(metadataBis);

        return map;
    }

    static QByteArray expectedSoapMessage()
    {
        return QByteArray(xmlEnvBegin11()) + " xmlns:wsa=\"http://www.w3.org/2005/08/addressing\""
               + " xmlns:n1=\"http://www.ecerami.com/wsdl/HelloService.wsdl\">"
               "<soap:Header>"
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
               "</soap:Body>" + xmlEnvEnd();
    }

    static QByteArray emptyResponse()
    {
        return QByteArray(xmlEnvBegin11()) + "><soap:Body/>";
    }
};

QTEST_MAIN(WSAddressingTest)

#include "wsaddressingtest.moc"
