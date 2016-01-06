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
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapAuthentication.h"
#include "wsdl_import_definition.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

class ImportDefinitionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testRequest()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExampleService service(this);
        service.setEndPoint(server.endPoint());

        // prepare instances
        CFWT__Authenticate req;
        req.setName("MyUser");
        req.setPhrase("Adsadsda asdasd asda");
        CFWT__MyAuthenticate auth;
        auth.setRequest(req);
        service.myAuthenticate(auth);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin11()) + ">"
            "<soap:Body>"
            "<n1:MyAuthenticate xmlns:n1=\"http://something.mydomain.com/types\">"
            "<request>"
            "<name>MyUser</name>"
            "<phrase>Adsadsda asdasd asda</phrase>"
            "</request>"
            "</n1:MyAuthenticate>"
            "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

private:
    static QByteArray queryResponse()
    {
        return QByteArray(xmlEnvBegin11()) + " xmlns:sf=\"urn:sobject.partner.soap.sforce.com\"><soap:Body>"
               "<queryResponse>" // TODO
               "</queryResponse>"
               "</soap:Body>" + xmlEnvEnd();
    }
};

QTEST_MAIN(ImportDefinitionTest)

#include "test_import_definition.moc"
