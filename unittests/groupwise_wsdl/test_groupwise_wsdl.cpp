#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapAuthentication.h"
#include "wsdl_groupwise.h"
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

class GroupwiseTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGeneratedMethods()
    {
        // No runtime test yet, just checking that the methods got generated
        if (false) { // Don't contact localhost:8080 :-)
            GroupwiseService::GroupWiseBinding groupwise;
            METHODS__AcceptRequest acceptRequest;
            acceptRequest.setComment(QString::fromLatin1("Comment"));
            METHODS__AcceptResponse response = groupwise.acceptRequest(acceptRequest);
            (void)response.status();
        }
    }

    // This test uses "internal" API, not very wise.
    // Makes refactoring harder.
    void testStringBaseType()
    {
        TYPES__ContainerRef cref(QString::fromLatin1("str"));
        cref.setDeleted(QDateTime(QDate(2010,31,12)));
        const KDSoapValue v = cref.serialize(QLatin1String("container"));

        TYPES__ContainerRef cref2;
        cref2.deserialize(v);
        QCOMPARE(cref.value(), cref2.value());
        QCOMPARE(cref.deleted(), cref2.deleted());
    }

    void testBase64()
    {
        HttpServerThread server(updateVersionStatusResponse(), HttpServerThread::Public);
        GroupwiseService::GroupWiseBinding groupwise;
        groupwise.setEndPoint(server.endPoint());

        METHODS__UpdateVersionStatusRequest req;
        req.setEvent(TYPES__VersionEventType::Archive);
        req.setId(QString::fromLatin1("TheId"));
        TYPES__SignatureData sigData;
        sigData.setData("ABCDEF");
        sigData.setSize(6);
        req.setPart(sigData);
        METHODS__UpdateVersionStatusResponse response = groupwise.updateVersionStatusRequest(req);
        //qDebug() << groupwise.lastError(); // TODO, should be set because the response was incorrect
        Q_UNUSED(response);

        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) +
            "><soap:Body>"
            "<n1:updateVersionStatusRequest xmlns:n1=\"http://schemas.novell.com/2005/01/GroupWise/groupwise.wsdl\">"
                "<n1:id>TheId</n1:id>"
                "<n1:event>archive</n1:event>"
                "<n1:part><n1:size>6</n1:size><n1:data>QUJDREVG</n1:data></n1:part>"
            "</n1:updateVersionStatusRequest>"
            "</soap:Body>" + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        //QCOMPARE(response.status().code(), 42); // TODO
    }

private:

    // Bogus response
    static QByteArray updateVersionStatusResponse() {
        return QByteArray(xmlEnvBegin) + " xmlns:gw=\"http://schemas.novell.com/2005/01/GroupWise/groupwise.wsdl\"><soap:Body>"
              "<queryResponse>"
               "<result>"
                "<done>true</done>"
                "<size>3</size>"
               "</result>"
              "</queryResponse>"
              "</soap:Body>" + xmlEnvEnd;
    }

};

QTEST_MAIN(GroupwiseTest)

#include "test_groupwise_wsdl.moc"
