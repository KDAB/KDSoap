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
                  "<n1:ResolveNames ReturnFullContactData=\"true\">"
                  "<n1:ParentFolderIds><n3:FolderId xmlns:n3=\"http://schemas.microsoft.com/exchange/services/2006/types\" Id=\"folderId\"/></n1:ParentFolderIds>"
                  "<n1:UnresolvedEntry/>"
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
                  "<n1:ResolveNames ReturnFullContactData=\"true\" xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
                  "<n1:UnresolvedEntry>test</n1:UnresolvedEntry>"
                "</n1:ResolveNames>"
                "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    T__TargetFolderIdType targetFolder()
    {
        T__TargetFolderIdType folder;
        T__DistinguishedFolderIdType distinguishedFolder;
        distinguishedFolder.setId(T__DistinguishedFolderIdNameType::Calendar);
        T__EmailAddressType mailbox;
        mailbox.setEmailAddress(QString("Chef@labex.fitformobility.de"));
        distinguishedFolder.setMailbox(mailbox);
        folder.setDistinguishedFolderId(distinguishedFolder);
        return folder;
    }

    void testSyncFolder()
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        T__TargetFolderIdType folder = targetFolder();

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
                "</n1:ItemShape>"
                "<n1:SyncFolderId>"
                  /* FolderId should not be there because it's a choice... */
                  "<n3:DistinguishedFolderId xmlns:n3=\"http://schemas.microsoft.com/exchange/services/2006/types\" Id=\"calendar\">"
                    "<n3:Mailbox>"
                      "<n3:EmailAddress>Chef@labex.fitformobility.de</n3:EmailAddress>"
                    "</n3:Mailbox>"
                  "</n3:DistinguishedFolderId>"
                "</n1:SyncFolderId>"
                "<n1:SyncState>MySyncState</n1:SyncState>"
                "<n1:MaxChangesReturned>10</n1:MaxChangesReturned>"
                "</n1:SyncFolderItems>"
                "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testGetFolder() // SOAP-87
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        TNS__GetFolderType request;

        //folder shape
        T__FolderResponseShapeType folderShape;
        folderShape.setBaseShape(T__DefaultShapeNamesType::IdOnly);
        T__PathToUnindexedFieldType pathToUnindexedField;
        pathToUnindexedField.setFieldURI(T__UnindexedFieldURIType::Folder_TotalCount);
        QList< QSharedPointer<T__BasePathToElementType> > paths;
        paths.append(QSharedPointer<T__BasePathToElementType>(new T__BasePathToElementType(pathToUnindexedField)));
        T__NonEmptyArrayOfPathsToElementType additionalProperties;
        additionalProperties.setPath(paths);
        folderShape.setAdditionalProperties(additionalProperties);
        request.setFolderShape(folderShape);

        //folder id
        T__DistinguishedFolderIdType distinguishedFolderId;
        distinguishedFolderId.setId(T__DistinguishedFolderIdNameType::Calendar);
        T__EmailAddressType mailbox;
        mailbox.setEmailAddress(QString("email@nowhere.com"));
        distinguishedFolderId.setMailbox(mailbox);
        QList<T__DistinguishedFolderIdType> distinguishedFolderIds;
        distinguishedFolderIds.append(distinguishedFolderId);
        T__NonEmptyArrayOfBaseFolderIdsType folderIds;
        folderIds.setDistinguishedFolderId(distinguishedFolderIds);
        request.setFolderIds(folderIds);

        //request folder
        service.getFolder(request);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin11()) + ">"
                "<soap:Body>"
                "<n1:GetFolder xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
                  "<n1:FolderShape>"
                    "<n2:BaseShape xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\">IdOnly</n2:BaseShape>"
                    "<n3:AdditionalProperties xmlns:n3=\"http://schemas.microsoft.com/exchange/services/2006/types\"><n3:Path/></n3:AdditionalProperties>"
                  "</n1:FolderShape>"
                  "<n1:FolderIds>"
                    "<n4:DistinguishedFolderId xmlns:n4=\"http://schemas.microsoft.com/exchange/services/2006/types\" Id=\"calendar\">"
                      "<n4:Mailbox>"
                        "<n4:EmailAddress>email@nowhere.com</n4:EmailAddress>"
                      "</n4:Mailbox>"
                    "</n4:DistinguishedFolderId>"
                  "</n1:FolderIds>"
                "</n1:GetFolder>"
                "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testCreateItem() // SOAP-93
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        TNS__CreateItemType request;
        T__TargetFolderIdType folder = targetFolder();
        request.setSavedItemFolderId(folder);
        T__NonEmptyArrayOfAllItemsType array;
        T__CalendarItemType item;
        item.setSubject("Subject");
        array.setCalendarItem(QList<T__CalendarItemType>() << item);
        T__MeetingRequestMessageType meetingRequest;
        T__TimeZoneType meetingTimeZone;
        meetingTimeZone.setTimeZoneName("W. Europe Standard Time");
        meetingRequest.setMeetingTimeZone(meetingTimeZone);
        array.setMeetingRequest(QList<T__MeetingRequestMessageType>() << meetingRequest);
        request.setItems(array);
        service.createItem(request);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin11()) + ">"
                "<soap:Body>"
                "<n1:CreateItem xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
                  "<n1:SavedItemFolderId>"
                    "<n2:DistinguishedFolderId xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\" Id=\"calendar\">"
                      "<n2:Mailbox>"
                        "<n2:EmailAddress>Chef@labex.fitformobility.de</n2:EmailAddress>"
                      "</n2:Mailbox>"
                    "</n2:DistinguishedFolderId>"
                  "</n1:SavedItemFolderId>"
                  "<n1:Items>"
                    "<n3:CalendarItem xmlns:n3=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
                      "<n3:Subject>Subject</n3:Subject>"
                    "</n3:CalendarItem>"
                    "<n4:MeetingRequest xmlns:n4=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
                      "<n4:MeetingTimeZone TimeZoneName=\"W. Europe Standard Time\"/>"
                    "</n4:MeetingRequest>"
                  "</n1:Items>"
                "</n1:CreateItem>"
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
