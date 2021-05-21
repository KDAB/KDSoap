/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "wsdl_import_definition.h"
#include "httpserver_p.h"
#include <QTest>
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
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin11())
            + ">"
              "<soap:Body>"
              "<n1:MyAuthenticate xmlns:n1=\"http://something.mydomain.com/types\">"
              "<request>"
              "<name>MyUser</name>"
              "<phrase>Adsadsda asdasd asda</phrase>"
              "</request>"
              "</n1:MyAuthenticate>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
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
};

QTEST_MAIN(ImportDefinitionTest)

#include "test_import_definition.moc"
