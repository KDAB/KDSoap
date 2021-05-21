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
#include "wsdl_logbookifv3.h"
#include "httpserver_p.h"
#include <QTest>
#include <QEventLoop>
#include <QDebug>

// https://www.elogbook.org/logbookws/logbookifv3.asmx

using namespace KDSoapUnitTestHelpers;

class LogbookTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testGetUpdateInfo()
    {
        HttpServerThread server(complexTypeResponse(), HttpServerThread::Public);
        LogbookIFV3::LogbookIFV3Soap12 service(this);
        service.setEndPoint(server.endPoint());

        TNS__GetUpdateInfo params;
        params.setDBSerial(123);
        params.setSpec(QString::fromLatin1("spec"));
        const TNS__GetUpdateInfoResponse response = service.getUpdateInfo(params);
        const TNS__GetUpdateInfoResult result = response.getUpdateInfoResult();
        const KDSoapValue val = result.any();
        // qDebug() << val;
        QCOMPARE(val.name(), QString::fromLatin1("success"));
        QCOMPARE(val.value().toString(), QString::fromLatin1("1"));

        // Check what we sent
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin12())
            + "><soap:Body>"
              "<n1:GetUpdateInfo xmlns:n1=\"https://www.elogbook.org/elogbook\">"
              "<n1:DBSerial>123</n1:DBSerial>"
              "<n1:spec>spec</n1:spec>"
              "</n1:GetUpdateInfo>"
              "</soap:Body>"
            + xmlEnvEnd() + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

private:
    static QByteArray complexTypeResponse()
    {
        // From https://www.elogbook.org/logbookws/logbookifv3.asmx?op=GetUpdateInfo
        return QByteArray(xmlEnvBegin12())
            + "><soap:Body>"
              "<GetUpdateInfoResponse xmlns=\"https://www.elogbook.org/elogbook\">"
              "<GetUpdateInfoResult><success>1</success></GetUpdateInfoResult>"
              "</GetUpdateInfoResponse>"
              "</soap:Body>"
            + xmlEnvEnd();
    }
};

QTEST_MAIN(LogbookTest)

#include "test_logbook_wsdl.moc"
