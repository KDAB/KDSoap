/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "wsdl_helloworldextended.h"
#include "httpserver_p.h"
#include <QtTest>
#include <QDebug>

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
        return QByteArray(xmlEnvBegin11()) + " xmlns:sf=\"urn:sobject.partner.soap.sforce.com\"><soap:Body>"
               "<queryResponse>" // TODO
               "</queryResponse>"
               "</soap:Body>" + xmlEnvEnd();
    }

    QByteArray expectedRequestXML()
    {
        return QByteArray(xmlEnvBegin11()) + "><soap:Body>"
               "<n1:SayHello xmlns:n1=\"http://www.ecerami.com/wsdl/HelloService.wsdl\" xsi:type=\"n1:SayHello\">"
               "<msgElement xsi:type=\"xsd:string\">Hello !</msgElement>"
               "<secondpartElement xsi:type=\"xsd:int\">42</secondpartElement>"
               "</n1:SayHello>"
               "</soap:Body>" + xmlEnvEnd()
               + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
};

QTEST_MAIN(MultipleInputParamTest)

#include "test_multiple_input_param.moc"

