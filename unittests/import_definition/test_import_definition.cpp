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
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapValue.h"
#include "httpserver_p.h"
#include "wsdl_import_definition.h"
#include <QDebug>
#include <QEventLoop>
#include <QTest>

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
