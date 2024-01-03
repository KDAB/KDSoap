/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "httpserver_p.h"
#include "wsdl_helloworldextended.h"
#include <QDebug>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

class MultipleInputParamTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testRequest()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        Hello_Service service(this);
        service.setEndPoint(server.endPoint());

        TNS__SayHello params;
        params.setMsgElement("Hello !");
        params.setSecondpartElement(42);

        service.sayHello(params);
        QCOMPARE(QString(server.receivedData()), QString(expectedRequestXML()));
    }

private:
    static QByteArray queryResponse()
    {
        return QByteArray(xmlEnvBegin11())
            + " xmlns:sf=\"urn:sobject.partner.soap.sforce.com\"><soap:Body>"
              "<queryResponse>" // TODO
              "</queryResponse>"
              "</soap:Body>"
            + xmlEnvEnd();
    }

    QByteArray expectedRequestXML()
    {
        return QByteArray(xmlEnvBegin11())
            + "><soap:Body>"
              "<n1:SayHello xmlns:n1=\"http://www.ecerami.com/wsdl/HelloService.wsdl\" xsi:type=\"n1:SayHello\">"
              "<msgElement xsi:type=\"xsd:string\">Hello !</msgElement>"
              "<secondpartElement xsi:type=\"xsd:int\">42</secondpartElement>"
              "</n1:SayHello>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
};

QTEST_MAIN(MultipleInputParamTest)

#include "test_multiple_input_param.moc"
