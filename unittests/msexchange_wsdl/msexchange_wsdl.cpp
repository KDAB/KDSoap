/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
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

class MSExchangeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        KDSoapUnitTestHelpers::initHashSeed();
    }

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

        T__RequestServerVersion requestServerVersion;
        requestServerVersion.setVersion(T__ExchangeVersionType(T__ExchangeVersionType::Exchange2007_SP1));
        service.setRequestVersionHeader(requestServerVersion);

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
            QByteArray(xmlEnvBegin11()) + " xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
                "<soap:Header>"
                  "<n2:ExchangeImpersonation xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\"><n2:ConnectingSID>"
                    "<n2:PrincipalName>dfaure</n2:PrincipalName>"
                    "<n2:SID>sid</n2:SID>"
                    "<n2:PrimarySmtpAddress>david.faure@kdab.com</n2:PrimarySmtpAddress>"
                  "</n2:ConnectingSID></n2:ExchangeImpersonation>"
                  "<n2:RequestServerVersion xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\" Version=\"Exchange2007_SP1\"/>"
                "</soap:Header>"
                "<soap:Body>"
                  "<n1:ResolveNames ReturnFullContactData=\"true\" SearchScope=\"ActiveDirectory\">"
                  "<n1:ParentFolderIds><n3:FolderId xmlns:n3=\"http://schemas.microsoft.com/exchange/services/2006/types\" Id=\"folderId\"/></n1:ParentFolderIds>"
                "</n1:ResolveNames>"
                "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testNoParentFolderIds()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        TNS__ResolveNamesType req;
        req.setReturnFullContactData(true);

        T__NonEmptyStringType unresolvedEntry;
        unresolvedEntry.setValue("test");
        req.setUnresolvedEntry(unresolvedEntry);

        service.resolveNames(req);

        // Check what we sent
        // If this contains <n1:ParentFolderIds/>, the MS Exchange server will reject it:
        // The element 'ParentFolderIds' in namespace '.../2006/messages' has incomplete content.
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin11()) + ">"
                "<soap:Body>"
                  "<n1:ResolveNames ReturnFullContactData=\"true\" SearchScope=\"ActiveDirectory\" xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
                  "<n1:UnresolvedEntry>test</n1:UnresolvedEntry>"
                "</n1:ResolveNames>"
                "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testSyncFolder()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        T__TargetFolderIdType folder;
        T__DistinguishedFolderIdType distinguishedFolder;
        distinguishedFolder.setId(T__DistinguishedFolderIdNameType::Calendar);
        T__EmailAddressType mailbox;
        mailbox.setEmailAddress(QString("Chef@labex.fitformobility.de")); //TODO replace with option
        distinguishedFolder.setMailbox(mailbox);
        folder.setDistinguishedFolderId(distinguishedFolder);

        //prepare call
        TNS__SyncFolderItemsType request;
        //item shape
        T__ItemResponseShapeType itemShape;
        itemShape.setBaseShape(T__DefaultShapeNamesType::Default);
        request.setItemShape(itemShape);
        request.setSyncFolderId(folder);
        //syncstate
        request.setSyncState("MySyncState");
        //max items
        request.setMaxChangesReturned(10);
        service.syncFolderItems(request);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin11()) + ">"
                "<soap:Body>"
                "<n1:SyncFolderItems xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
                "<n1:ItemShape>"
                "<n2:BaseShape xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\">Default</n2:BaseShape>"
                "<n3:IncludeMimeContent xmlns:n3=\"http://schemas.microsoft.com/exchange/services/2006/types\">false</n3:IncludeMimeContent>"
                "<n4:BodyType xmlns:n4=\"http://schemas.microsoft.com/exchange/services/2006/types\">Best</n4:BodyType>"
                "</n1:ItemShape>"
                "<n1:SyncFolderId>"
                  "<n5:DistinguishedFolderId xmlns:n5=\"http://schemas.microsoft.com/exchange/services/2006/types\" Id=\"calendar\">"
                    "<n5:Mailbox>"
                      "<n5:EmailAddress>Chef@labex.fitformobility.de</n5:EmailAddress>"
                      "<n5:MailboxType>Mailbox</n5:MailboxType>"
                    "</n5:Mailbox>"
                  "</n5:DistinguishedFolderId>"
                "</n1:SyncFolderId>"
                "<n1:SyncState>MySyncState</n1:SyncState>"
                "<n1:MaxChangesReturned>10</n1:MaxChangesReturned>"
                "</n1:SyncFolderItems>"
                "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

private:
    static QByteArray queryResponse() {
        return QByteArray(xmlEnvBegin11()) + " xmlns:sf=\"urn:sobject.partner.soap.sforce.com\"><soap:Body>"
              "<queryResponse>" // TODO
              "</queryResponse>"
              "</soap:Body>" + xmlEnvEnd();
    }
};

QTEST_MAIN(MSExchangeTest)

#include "msexchange_wsdl.moc"
