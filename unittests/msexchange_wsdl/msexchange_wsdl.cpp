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

Q_DECLARE_METATYPE(TNS__GetFolderType)

class MSExchangeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
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
        service.setImpersonationHeader(impersonation);

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
        T__PathToUnindexedFieldType *pathToUnindexedField = new T__PathToUnindexedFieldType; // ownership will transfer to QSharedPointer
        pathToUnindexedField->setFieldURI(T__UnindexedFieldURIType::Folder_TotalCount);
        QList< QSharedPointer<T__BasePathToElementType> > paths;
        paths.append(QSharedPointer<T__BasePathToElementType>(pathToUnindexedField));
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
            "<n3:AdditionalProperties xmlns:n3=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
            "<n3:FieldURI FieldURI=\"folder:TotalCount\"/>"
            "</n3:AdditionalProperties>"
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
        T__AcceptItemType acceptItem;
        array.setAcceptItem(QList<T__AcceptItemType>() << acceptItem);
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
            "<n5:AcceptItem xmlns:n5=\"http://schemas.microsoft.com/exchange/services/2006/types\"/>"
            "</n1:Items>"
            "</n1:CreateItem>"
            "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testUpdateItem() // SOAP-97
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        TNS__UpdateItemType request;
        T__TargetFolderIdType folder = targetFolder();
        request.setSavedItemFolderId(folder);

        T__ItemChangeType itemChange;

        T__SetItemFieldType setItemField;
        T__PathToUnindexedFieldType path;
        path.setFieldURI(T__UnindexedFieldURIType::Item_Subject);
        setItemField.setPath(path);
        T__CalendarItemType calendarItem;
        calendarItem.setSubject("newSubject");
        setItemField.setCalendarItem(calendarItem);

        T__NonEmptyArrayOfItemChangeDescriptionsType updates;
        updates.setSetItemField(QList<T__SetItemFieldType>() << setItemField);
        itemChange.setUpdates(updates);

        T__NonEmptyArrayOfItemChangesType array;
        array.setItemChange(QList<T__ItemChangeType>() << itemChange);
        request.setItemChanges(array);
        service.updateItem(request);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin11()) + ">"
            "<soap:Body>"
            "<n1:UpdateItem ConflictResolution=\"NeverOverwrite\" xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
            "<n1:SavedItemFolderId>"
            "<n2:DistinguishedFolderId xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\" Id=\"calendar\">"
            "<n2:Mailbox>"
            "<n2:EmailAddress>Chef@labex.fitformobility.de</n2:EmailAddress>"
            "</n2:Mailbox>"
            "</n2:DistinguishedFolderId>"
            "</n1:SavedItemFolderId>"
            "<n1:ItemChanges>"
            "<n3:ItemChange xmlns:n3=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
            "<n3:Updates>"
            "<n3:SetItemField>"
            "<n3:FieldURI FieldURI=\"item:Subject\"/>"
            "<n3:CalendarItem>"
            "<n3:Subject>newSubject</n3:Subject>"
            "</n3:CalendarItem>"
            "</n3:SetItemField>"
            "</n3:Updates>"
            "</n3:ItemChange>"
            "</n1:ItemChanges>"
            "</n1:UpdateItem>"
            "</soap:Body>" + xmlEnvEnd()
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testServerVersionInfo() // SOAP-116
    {
        QByteArray serverResponseXml =
            "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
            "<s:Header>"
            "<h:ServerVersionInfo MajorVersion=\"15\" MinorVersion=\"0\" MajorBuildNumber=\"995\" MinorBuildNumber=\"28\" Version=\"V2_15\" xmlns:h=\"http://schemas.microsoft.com/exchange/services/2006/types\" xmlns=\"http://schemas.microsoft.com/exchange/services/2006/types\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"/>"
            "</s:Header>"
            "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
            "<m:SyncFolderItemsResponse xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
            "<m:ResponseMessages>"
            "<m:SyncFolderItemsResponseMessage ResponseClass=\"Success\">"
            "<m:ResponseCode>NoError</m:ResponseCode>"
            "<m:SyncState>H4sIAAAAAAAEAGNgYGcAAotqE0tHE2NTA0ddZ3NHC10TR2djXSdnJ2ddNyMnC2cnczdLU1OD2vBgveDKvOTgksSSVOfEvMSiSgYr0nW65eekpBZ5pjBYkq43LLWoODM/j8GaaK3+QMuKS4JSk1Mzy1JTQjJzU0nwrU9icYlnXnFJYl5yqncqKb71zS9K9SxJzS32zwtOLSpLLSLByXDfhgNxUW5iUTYklrgYGISA0tDwAxkOUskgCJQyAGI9kJqwW2wmEwW3eq1Q4DWONF+xh5HBY/WkZAUxe7elE6ZFHb2QsgSoiL2ukIGBkYGPgRmkhZtBPOh3mQjTJQ8GIaAoLxADratiZGDwdQzw9HX0AylicDN1CwMrRwOtQFyBxP8LxNZY1FkAzTND4h9pWa2bq7XWtWXCjw3TL5y/CDI7COiqq+d0AhjuhbDvVqvn8J2s/69qrtKPvWxgLYwMdy6UycUyyHnvb5dd9O3xSrMgqLh5t+3sbxhGwmQZGICGMtw+bWlxXmWD/4wd29SnTd+1HC671ef0M4xQQuidI7AMw2RWoPjFOU4CjYymTnZznTzmBzDcTfxyXjSuyadr1dFH2h2bvrMx7FiaXtJGwFG4PGr7krEBANRZ2yGcAwAA</m:SyncState>"
            "<m:IncludesLastItemInRange>true</m:IncludesLastItemInRange>"
            "<m:Changes>"
            "<t:Create>"
            "<t:Message>"
            "<t:ItemId Id=\"AAMkAGNiY2YxMjY3LTUxYjgtNGI1Yy1hOTM2LTU4MTM5OTZiNjdjYgBGAAAAAABW2gY0kRG1SqggDTNZN6i8BwBIq5JjIBY/RqWQllrF0GSkAAAAB35xAADdYfTPFV6CTIqqxeIriLL3ALilZ3SGAAA=\" ChangeKey=\"CQAAABYAAADEhKstbSqtRYSQ+LCX0M/RAAAAY37E\"/>"
            "<t:Subject>Freedom</t:Subject>"
            "<t:Sensitivity>Normal</t:Sensitivity>"
            "<t:Size>14070</t:Size>"
            "<t:DateTimeSent>2014-10-02T13:24:47Z</t:DateTimeSent>"
            "<t:DateTimeCreated>2014-10-02T13:24:49Z</t:DateTimeCreated>"
            "<t:HasAttachments>false</t:HasAttachments>"
            "<t:From>"
            "<t:Mailbox>"
            "<t:Name>Simon Hain</t:Name>"
            "<t:EmailAddress>Simon.Hain@isec7.com</t:EmailAddress>"
            "<t:RoutingType>SMTP</t:RoutingType>"
            "</t:Mailbox>"
            "</t:From>"
            "<t:IsRead>true</t:IsRead>"
            "</t:Message>"
            "</t:Create>"
            "</m:Changes>"
            "</m:SyncFolderItemsResponseMessage>"
            "</m:ResponseMessages>"
            "</m:SyncFolderItemsResponse>"
            "</s:Body>"
            "</s:Envelope>";

        HttpServerThread server(serverResponseXml, HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        GetFolderJob *job = new GetFolderJob(&service);
        TNS__GetFolderType request;
        T__FolderResponseShapeType folderShape;
        folderShape.setBaseShape(T__DefaultShapeNamesType::IdOnly);
        request.setFolderShape(folderShape);

        T__NonEmptyArrayOfBaseFolderIdsType folderIds;
        QList<T__DistinguishedFolderIdType> distinguishedFolderIds;
        T__DistinguishedFolderIdType distinguishedFolder;
        distinguishedFolder.setId(T__DistinguishedFolderIdNameType::Inbox);
        distinguishedFolderIds.append(distinguishedFolder);
        folderIds.setDistinguishedFolderId(distinguishedFolderIds);
        request.setFolderIds(folderIds);

        QEventLoop loop;
        QObject::connect(job, SIGNAL(finished(KDSoapJob*)), &loop, SLOT(quit()));
        job->setRequest(request);
        job->start();
        loop.exec();

        // comapring with "<h:ServerVersionInfo MajorVersion=\"15\" MinorVersion=\"0\" MajorBuildNumber=\"995\" MinorBuildNumber=\"28\" Version=\"V2_15\" [...]
        QCOMPARE(job->serverVersion().majorVersion(), 15);
        QCOMPARE(job->serverVersion().majorBuildNumber(), 995);
        QCOMPARE(job->serverVersion().minorVersion(), 0);
        QCOMPARE(job->serverVersion().minorBuildNumber(), 28);
        QCOMPARE(job->serverVersion().version(), QString("V2_15"));
    }

    void testCharactersSequenceValidOrInvalid_data() // We use a token within <subject> to later replace by a valid or invalid characters
    {
        const QString serverResponseXML = "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
                                          "<s:Header>"
                                          "<h:ServerVersionInfo MajorVersion=\"15\" MinorVersion=\"0\" MajorBuildNumber=\"995\" MinorBuildNumber=\"28\" Version=\"V2_15\" xmlns:h=\"http://schemas.microsoft.com/exchange/services/2006/types\" xmlns=\"http://schemas.microsoft.com/exchange/services/2006/types\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"/>"
                                          "</s:Header>"
                                          "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                          "<m:SyncFolderItemsResponse xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
                                          "<m:ResponseMessages>"
                                          "<m:SyncFolderItemsResponseMessage ResponseClass=\"Success\">"
                                          "<m:ResponseCode>NoError</m:ResponseCode>"
                                          "<m:SyncState>H4sIAAAAAAAEAGNgYGcAAotqE0tHE2NTA0ddZ3NHC10TR2djXSdnJ2ddNyMnC2cnczdLU1OD2vBgveDKvOTgksSSVOfEvMSiSgYr0nW65eekpBZ5pjBYkq43LLWoODM/j8GaaK3+QMuKS4JSk1Mzy1JTQjJzU0nwrU9icYlnXnFJYl5yqncqKb71zS9K9SxJzS32zwtOLSpLLSLByXDfhgNxUW5iUTYklrgYGISA0tDwAxkOUskgCJQyAGI9kJqwW2wmEwW3eq1Q4DWONF+xh5HBY/WkZAUxe7elE6ZFHb2QsgSoiL2ukIGBkYGPgRmkhZtBPOh3mQjTJQ8GIaAoLxADratiZGDwdQzw9HX0AylicDN1CwMrRwOtQFyBxP8LxNZY1FkAzTND4h9pWa2bq7XWtWXCjw3TL5y/CDI7COiqq+d0AhjuhbDvVqvn8J2s/69qrtKPvWxgLYwMdy6UycUyyHnvb5dd9O3xSrMgqLh5t+3sbxhGwmQZGICGMtw+bWlxXmWD/4wd29SnTd+1HC671ef0M4xQQuidI7AMw2RWoPjFOU4CjYymTnZznTzmBzDcTfxyXjSuyadr1dFH2h2bvrMx7FiaXtJGwFG4PGr7krEBANRZ2yGcAwAA</m:SyncState>"
                                          "<m:IncludesLastItemInRange>true</m:IncludesLastItemInRange>"
                                          "<m:Changes>"
                                          "<t:Create>"
                                          "<t:Message>"
                                          "<t:ItemId Id=\"AAMkAGNiY2YxMjY3LTUxYjgtNGI1Yy1hOTM2LTU4MTM5OTZiNjdjYgBGAAAAAABW2gY0kRG1SqggDTNZN6i8BwBIq5JjIBY/RqWQllrF0GSkAAAAB35xAADdYfTPFV6CTIqqxeIriLL3ALilZ3SGAAA=\" ChangeKey=\"CQAAABYAAADEhKstbSqtRYSQ+LCX0M/RAAAAY37E\"/>"
                                          "<t:Subject>subject %1</t:Subject>"
                                          "<t:Sensitivity>Normal</t:Sensitivity>"
                                          "<t:Size>14070</t:Size>"
                                          "<t:DateTimeSent>2014-10-02T13:24:47Z</t:DateTimeSent>"
                                          "<t:DateTimeCreated>2014-10-02T13:24:49Z</t:DateTimeCreated>"
                                          "<t:HasAttachments>false</t:HasAttachments>"
                                          "<t:From>"
                                          "<t:Mailbox>"
                                          "<t:Name>Simon Hain</t:Name>"
                                          "<t:EmailAddress>Simon.Hain@isec7.com</t:EmailAddress>"
                                          "<t:RoutingType>SMTP</t:RoutingType>"
                                          "</t:Mailbox>"
                                          "</t:From>"
                                          "<t:IsRead>true</t:IsRead>"
                                          "</t:Message>"
                                          "</t:Create>"
                                          "</m:Changes>"
                                          "</m:SyncFolderItemsResponseMessage>"
                                          "</m:ResponseMessages>"
                                          "</m:SyncFolderItemsResponse>"
                                          "</s:Body>"
                                          "</s:Envelope>";

        TNS__GetFolderType request;
        T__FolderResponseShapeType folderShape;
        folderShape.setBaseShape(T__DefaultShapeNamesType::IdOnly);
        request.setFolderShape(folderShape);

        T__NonEmptyArrayOfBaseFolderIdsType folderIds;
        QList<T__DistinguishedFolderIdType> distinguishedFolderIds;
        T__DistinguishedFolderIdType distinguishedFolder;
        distinguishedFolder.setId(T__DistinguishedFolderIdNameType::Inbox);
        distinguishedFolderIds.append(distinguishedFolder);
        folderIds.setDistinguishedFolderId(distinguishedFolderIds);
        request.setFolderIds(folderIds);

        QTest::addColumn<QString>("serverResponseXml");
        QTest::addColumn<TNS__GetFolderType>("request");
        QTest::addColumn<QString>("expectedResult");

        QTest::newRow("invalid 13") << serverResponseXML.arg("&#x13;") << request << QString("subject ?");
        QTest::newRow("valid A") << serverResponseXML.arg("&#x41;") << request << QString("subject A");
        QTest::newRow("valid B") << serverResponseXML.arg("&#x42;") << request << QString("subject B");
        QTest::newRow("valid_single_quote") << serverResponseXML.arg("&#039;") << request << QString("subject '");
    }

    void testCharactersSequenceValidOrInvalid() // SOAP-113: invalid XML character within <subject>
    {
        QFETCH(QString, serverResponseXml);
        QFETCH(TNS__GetFolderType, request);
        QFETCH(QString, expectedResult);;

        HttpServerThread server(serverResponseXml.toUtf8(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());
        GetFolderJob *job = new GetFolderJob(&service);

        QEventLoop loop;
        QObject::connect(job, SIGNAL(finished(KDSoapJob*)), &loop, SLOT(quit()));
        job->setRequest(request);
        job->start();
        loop.exec();

        QVERIFY2(!job->isFault(), qPrintable(job->faultAsString()));

        TNS__GetFolderResponseType folderResult  = job->getFolderResult();
        TNS__ArrayOfResponseMessagesType respMessages = folderResult.responseMessages();
        QList< TNS__SyncFolderItemsResponseMessageType > syncFolderRespList = respMessages.syncFolderItemsResponseMessage();
        QCOMPARE(syncFolderRespList.size(), 1);

        T__SyncFolderItemsChangesType syncFolderChange = syncFolderRespList.first().changes();
        QList< T__SyncFolderItemsCreateOrUpdateType > syncFolderChangeCreate = syncFolderChange.create();
        QCOMPARE(syncFolderChangeCreate.size(), 1);

        T__MessageType message = syncFolderChangeCreate.first().message();
        //qDebug() << message.subject();
        QCOMPARE(message.itemId().id(), QString("AAMkAGNiY2YxMjY3LTUxYjgtNGI1Yy1hOTM2LTU4MTM5OTZiNjdjYgBGAAAAAABW2gY0kRG1SqggDTNZN6i8BwBIq5JjIBY/RqWQllrF0GSkAAAAB35xAADdYfTPFV6CTIqqxeIriLL3ALilZ3SGAAA="));
        QCOMPARE(message.subject(), expectedResult);  // We are supposed to have replace invalid characters
    }

    void testAttachmentMessage() // SOAP-111
    {
        HttpServerThread server(queryResponse(), HttpServerThread::Public);
        ExchangeServices service(this);
        service.setEndPoint(server.endPoint());

        TNS__CreateAttachmentType request;
        T__ItemIdType itemId;
        itemId.setId("id");
        itemId.setChangeKey("changeKey");
        request.setParentItemId(itemId);
        T__NonEmptyArrayOfAttachmentsType attachmentsType;
        QList<T__FileAttachmentType> attachments;
        T__FileAttachmentType attachment;
        attachment.setName("fileName");
        attachment.setContent(QByteArray("content"));
        attachments.append(attachment);

        attachmentsType.setFileAttachment(attachments);
        request.setAttachments(attachmentsType);

        service.createAttachment(request);

        //qDebug() << "received data" << server.receivedData();

        // Check what we sent
        QByteArray expectedRequestXml = QByteArray(xmlEnvBegin11()) + ">" +
                                        "<soap:Body>"
                                        "<n1:CreateAttachment xmlns:n1=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
                                        "<n1:ParentItemId Id=\"id\" ChangeKey=\"changeKey\"/>"
                                        "<n1:Attachments>"
                                        "<n2:FileAttachment xmlns:n2=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
                                        "<n2:Name>fileName</n2:Name>"
                                        "<n2:Content>Y29udGVudA==</n2:Content>"
                                        "</n2:FileAttachment>"
                                        "</n1:Attachments>"
                                        "</n1:CreateAttachment>"
                                        "</soap:Body>"
                                        + + xmlEnvEnd()
                                        + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    void testAttributeGroup() // SOAP-117
    {
//        <xs:attributeGroup name="FindResponsePagingAttributes">
//          <xs:attribute name="IncludesLastItemInRange" type="xs:boolean" use="optional"/> [...]
//        </xs:attributeGroup>

        T__FindItemParentType fipt; // has attributes from a AttributeGroup tag
        fipt.setAbsoluteDenominator(42);
        fipt.setNumeratorOffset(42);
        fipt.setIncludesLastItemInRange(true);

        QCOMPARE(fipt.absoluteDenominator(), 42);
        QCOMPARE(fipt.numeratorOffset(), 42);
        QVERIFY(fipt.includesLastItemInRange());
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

QTEST_MAIN(MSExchangeTest)

#include "msexchange_wsdl.moc"
