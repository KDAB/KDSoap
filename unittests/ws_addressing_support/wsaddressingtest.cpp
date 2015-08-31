/****************************************************************************
** Copyright (C) 2010-2014 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

public:
    WSAddressingTest(){}
    virtual ~WSAddressingTest() {}

private Q_SLOTS:
    void shouldWriteAProperSoapMessageWithRightsAddressingProperties()
    {
        // GIVEN
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), "http://www.ecerami.com/wsdl/HelloService");

        KDSoapMessage message;
        KDSoapMessageAddressingProperties map;

        QString none = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::None);
        QCOMPARE(none, QString("http://www.w3.org/2005/08/addressing/none"));

        QString anonymous = KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous);
        QCOMPARE(anonymous, QString("http://www.w3.org/2005/08/addressing/anonymous"));

        // with some message addressing properties
        map.setAction("sayHello");
        map.setDestination("http://www.ecerami.com/wsdl/HelloService");
        map.setSourceEndpoint("http://www.ecerami.com/wsdl/source");
        map.setFaultEndpoint("http://www.ecerami.com/wsdl/fault");
        map.setMessageID("uuid:e197db59-0982-4c9c-9702-4234d204f7f4");
        map.setReplyEndpoint(KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous));

//      NOT SUPPORTED ATM
        //map.setRelationship();
        //map.setReferenceParameters();

        // and with content request
        const QString action = QString::fromLatin1("sayHello");
        message.setUse(KDSoapMessage::EncodedUse);
        KDSoapValue valueMsg(QString::fromLatin1("msg"), QVariant::fromValue(QString("HelloContentMessage")), KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("string"));
        valueMsg.setNamespaceUri(QString::fromLatin1("http://www.ecerami.com/wsdl/HelloService.wsdl"));
        message = valueMsg;

        message.setMessageAddressingProperties(map); // TODO: uncomment when created within KDSoapMessage

        // WHEN
        KDSoapMessage reply = client.call(QLatin1String("sayHello"), message, action);
        // qDebug() << "--- reply received to client" << reply.toXml() << "\n"; // TODO : check how is the server currently responding to this

        // THEN
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedSoapMessage()));
    }

    void shouldRecognizeWSAddressingFeatureFromWSDL()
    {
        HttpServerThread server(QByteArray(), HttpServerThread::Public);
        Hello_Service service;
        service.setEndPoint(server.endPoint());
        service.sayHello("Hi !");

        // TODO: all these properties should match wsdl and or default value
        // following specifications

//        <wsa:To>xs:anyURI</wsa:To> ?
//        <wsa:From>wsa:EndpointReferenceType</wsa:From> ?
//        <wsa:ReplyTo>wsa:EndpointReferenceType</wsa:ReplyTo> ?
//        <wsa:FaultTo>wsa:EndpointReferenceType</wsa:FaultTo> ?
//        <wsa:Action>xs:anyURI</wsa:Action>
//        <wsa:MessageID>xs:anyURI</wsa:MessageID> ?
//        <wsa:RelatesTo RelationshipType="xs:anyURI"?>xs:anyURI</wsa:RelatesTo> *
//        <wsa:ReferenceParameters>xs:any*</wsa:ReferenceParameters> ?

        // NEED TO add implicit / explicit recognition
    }
private:
        static QByteArray expectedSoapMessage() {
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
                    "</soap:Header>"
                    "<soap:Body>"
                      "<n1:sayHello xsi:type=\"xsd:string\">HelloContentMessage</n1:sayHello>"
                    "</soap:Body>" + xmlEnvEnd();
        }

        static QByteArray emptyResponse() {
            return QByteArray(xmlEnvBegin11()) + "><soap:Body/>";
        }
};

QTEST_MAIN(WSAddressingTest)

#include "wsaddressingtest.moc"
