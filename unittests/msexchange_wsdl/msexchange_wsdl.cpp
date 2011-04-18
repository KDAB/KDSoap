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
        " xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\""
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
                  "<n1:ExchangeImpersonation><n1:ConnectingSID><n1:PrincipalName>dfaure</n1:PrincipalName><n1:SID>sid</n1:SID><n1:PrimarySmtpAddress>david.faure@kdab.com</n1:PrimarySmtpAddress></n1:ConnectingSID></n1:ExchangeImpersonation>"
                "</soap:Header>"
                "<soap:Body>"
                  "<n1:ResolveNames n1:ReturnFullContactData=\"true\" n1:SearchScope=\"ActiveDirectory\">"
                  "<n1:ParentFolderIds><n1:FolderId n1:Id=\"folderId\" n1:ChangeKey=\"\"/></n1:ParentFolderIds>"
                  "<n1:UnresolvedEntry/>"
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
