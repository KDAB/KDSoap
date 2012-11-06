/****************************************************************************
** Copyright (C) 2010-2012 Klaralvdalens Datakonsult AB.  All rights reserved.
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
#include "wsdl_Services.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

static const char* xmlEnvBegin =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<soap:Envelope"
        " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
        " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"";
static const char* xmlEnvEnd = "</soap:Envelope>";

class MSExchangeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testExchangeMessage()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        TNS__ResolveNamesType req;
        req.setReturnFullContactData(true);
        T__NonEmptyArrayOfBaseFolderIdsType folderIds;
        T__FolderIdType folderId;
        folderId.setId(QString::fromLatin1("folderId"));
        QList<T__FolderIdType> folderIdList;
        folderIdList << folderId;
        folderIds.setFolderId(folderIdList);
        req.setParentFolderIds(folderIds);

        T__ExchangeImpersonationType impersonation;
        T__ConnectingSIDType sid;
        sid.setPrincipalName(QString::fromLatin1("dfaure"));
        sid.setPrimarySmtpAddress(QString::fromLatin1("david.faure@kdab.com"));
        sid.setSID(QString::fromLatin1("sid"));
        impersonation.setConnectingSID(sid);
        service.setImpersonationHeader( impersonation );

        service.resolveNames(req);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) + " xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
                "<soap:Header>"
                  "<n2:ExchangeImpersonation xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\"><n2:ConnectingSID>"
                    "<n2:PrincipalName>dfaure</n2:PrincipalName>"
                    "<n2:SID>sid</n2:SID>"
                    "<n2:PrimarySmtpAddress>david.faure@kdab.com</n2:PrimarySmtpAddress>"
                  "</n2:ConnectingSID></n2:ExchangeImpersonation>"
                "</soap:Header>"
                "<soap:Body>"
                  "<n1:ResolveNames n1:ReturnFullContactData=\"true\" n1:SearchScope=\"ActiveDirectory\">"
                  "<n1:ParentFolderIds><n2:FolderId xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\" n2:Id=\"folderId\" n2:ChangeKey=\"\" xsi:nil=\"true\"/></n1:ParentFolderIds>"
                  "<n1:UnresolvedEntry xsi:nil=\"true\"/>"
                "</n1:ResolveNames>"
                "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

private:
    static QByteArray queryResponse() {
        return QByteArray(xmlEnvBegin) + " xmlns:sf=\"urn:sobject.partner.soap.sforce.com\"><soap:Body>"
              "<queryResponse>" // TODO
              "</queryResponse>"
              "</soap:Body>" + xmlEnvEnd;
    }
};

QTEST_MAIN(MSExchangeTest)

#include "msexchange_wsdl.moc"
