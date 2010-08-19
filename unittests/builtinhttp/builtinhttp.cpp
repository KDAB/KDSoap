#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapAuthentication.h"
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

class BuiltinHttpTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // Test that we can use asyncCall without using a watcher, just waiting and checking isFinished.
    void testAsyncCall()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapPendingCall call = client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!call.isFinished());
        QTest::qWait(1000);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
#if QT_VERSION >= 0x040600
        QVERIFY(call.isFinished());
#endif
        QCOMPARE(call.returnMessage().arguments().child(QLatin1String("employeeCountry")).value().toString(), QString::fromLatin1("France"));
    }

    void testFault()
    {
        HttpServerThread server(QByteArray(), HttpServerThread::Public | HttpServerThread::Error404);
        KDSoapClientInterface client(server.endPoint(), QString::fromLatin1("urn:msg"));
        KDSoapMessage message;
        KDSoapMessage ret = client.call(QLatin1String("Method1"), message);
        QVERIFY(ret.isFault());
        QCOMPARE(ret.faultAsString(), QString::fromLatin1(
                     "Fault code: 203\n"
                     "Fault description: Error downloading %1 - server replied: Not Found ()").arg(server.endPoint()));
    }

    // Test for basic auth, with async call
    void testAsyncCallWithAuth()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::BasicAuth);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("testpass"));
        client.setAuthentication(auth);
        KDSoapPendingCall call = client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!call.isFinished());
        waitForCallFinished(call);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
#if QT_VERSION >= 0x040600
        QVERIFY(call.isFinished());
#endif
        QCOMPARE(call.returnMessage().arguments().child(QLatin1String("employeeCountry")).value().toString(), QString::fromLatin1("France"));
    }

    // Test for refused auth, with async call
    void testAsyncCallRefusedAuth()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::BasicAuth);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("invalid"));
        client.setAuthentication(auth);
        KDSoapPendingCall call = client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!call.isFinished());
        waitForCallFinished(call);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
#if QT_VERSION >= 0x040600
        QVERIFY(call.isFinished());
#endif
        QVERIFY(call.returnMessage().isFault());
    }

    // Test for refused auth, with sync call (i.e. in thread)
    void testCallRefusedAuth()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::BasicAuth);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("invalid"));
        client.setAuthentication(auth);
        KDSoapMessage reply = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(reply.isFault());
    }

    void testCorrectHttpHeader()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("unused"));
        client.setAuthentication(auth); // unused...

        QByteArray expectedRequestXml = expectedCountryRequest();
        client.setSoapVersion(KDSoapClientInterface::SOAP1_1);
        {
            KDSoapMessage ret = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
            // Check what we sent
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QVERIFY(!ret.isFault());
            QCOMPARE(ret.arguments().child(QLatin1String("employeeCountry")).value().toString(), QString::fromLatin1("France"));
        }        
        client.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        {
            KDSoapMessage ret = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
            // Check what we sent
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QVERIFY(!ret.isFault());
            QCOMPARE(ret.arguments().child(QLatin1String("employeeCountry")).value().toString(), QString::fromLatin1("France"));
        }
    }
    // Using direct call(), check the xml we send, the response parsing.
    // Then test callNoReply, then various ways to use asyncCall.
    void testCallNoReply()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::Public);

        // First, make the proper call
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapAuthentication auth;
        auth.setUser(QLatin1String("kdab"));
        auth.setPassword(QLatin1String("unused"));
        client.setAuthentication(auth); // unused...
        QByteArray expectedRequestXml = expectedCountryRequest();

        {
            KDSoapMessage ret = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
            // Check what we sent
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QVERIFY(!ret.isFault());
            QCOMPARE(ret.arguments().child(QLatin1String("employeeCountry")).value().toString(), QString::fromLatin1("France"));
        }

        // Now make the call again, but async, and don't wait for response.
        server.resetReceivedBuffers();
        //qDebug() << "== now calling callNoReply ==";
        client.callNoReply(QLatin1String("getEmployeeCountry"), countryMessage());
        QTest::qWait(200);
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        // What happens if we use asyncCall and discard the result?
        // The KDSoapPendingCall goes out of scope, and the network request is aborted.
        //
        // The whole reason KDSoapPendingCall is a value, is so that people don't forget
        // to delete it. But of course if they even forget to store it, nothing happens.
        server.resetReceivedBuffers();
        {
            client.asyncCall(QLatin1String("getEmployeeCountry"), countryMessage());
            QTest::qWait(200);
        }
        QVERIFY(server.receivedData().isEmpty());
    }

    void testRequestXml() // this tests the serialization of KDSoapValue[List] in KDSoapClientInterface
    {
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        KDSoapMessage message;
        message.setUse(KDSoapMessage::EncodedUse); // write out types explicitely

        // Test simpletype element
        message.addArgument(QString::fromLatin1("testString"), QString::fromUtf8("Hello Klarälvdalens"));

        // Test simpletype extended to have an attribute
        KDSoapValue val(QString::fromLatin1("val"), 5, countryMessageNamespace(), QString::fromLatin1("MyVal"));
        val.childValues().attributes().append(KDSoapValue(QString::fromLatin1("attr"), QString::fromLatin1("attrValue")));
        message.arguments().append(val);

        // Test complextype with child elements and attributes
        KDSoapValueList valueList;
        KDSoapValue orderperson(QString::fromLatin1("orderperson"), QString::fromLatin1("someone"), countryMessageNamespace(), QString::fromLatin1("Person"));
        valueList.append(orderperson);
        valueList.attributes().append(KDSoapValue(QString::fromLatin1("attr"), QString::fromLatin1("attrValue")));
        KDSoapValue order(QString::fromLatin1("order"), valueList, countryMessageNamespace(), QString::fromLatin1("MyOrder"));
        message.arguments().append(order);

        const QString XMLSchemaNS = QString::fromLatin1("http://www.w3.org/2001/XMLSchema"); // TODO namespace repository

        // Test array
        KDSoapValueList arrayContents;
        arrayContents.setArrayType(XMLSchemaNS, QString::fromLatin1("string"));
        arrayContents.append(KDSoapValue(QString::fromLatin1("item"), QString::fromLatin1("kdab"), XMLSchemaNS, QString::fromLatin1("string")));
        arrayContents.append(KDSoapValue(QString::fromLatin1("item"), QString::fromLatin1("rocks"), XMLSchemaNS, QString::fromLatin1("string")));
        KDSoapValue array(QString::fromLatin1("testArray"), arrayContents, QString::fromLatin1("http://schemas.xmlsoap.org/soap/encoding/"), QString::fromLatin1("Array"));
        message.arguments().append(array);

        // Add a header
        KDSoapMessage header1;
        header1.setUse(KDSoapMessage::EncodedUse);
        header1.addArgument(QString::fromLatin1("header1"), QString::fromLatin1("headerValue"));
        KDSoapHeaders headers;
        headers << header1;

        client.call(QLatin1String("test"), message, QString::fromLatin1("MySoapAction"), headers);
        // Check what we sent
        QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) +
            " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\""
            "><soap:Header>"
            "<n1:header1 xsi:type=\"xsd:string\">headerValue</n1:header1>"
            "</soap:Header>";
        const QByteArray expectedRequestBody =
            QByteArray("<soap:Body>"
            "<n1:test>"
            "<n1:testString xsi:type=\"xsd:string\">Hello Klarälvdalens</n1:testString>"
            "<n1:val xsi:type=\"n1:MyVal\" n1:attr=\"attrValue\">5</n1:val>"
            "<n1:order xsi:type=\"n1:MyOrder\" n1:attr=\"attrValue\"><n1:orderperson xsi:type=\"n1:Person\">someone</n1:orderperson></n1:order>"
            "<n1:testArray xsi:type=\"soap-enc:Array\" soap-enc:arrayType=\"xsd:string[2]\">"
             "<n1:item xsi:type=\"xsd:string\">kdab</n1:item>"
             "<n1:item xsi:type=\"xsd:string\">rocks</n1:item>"
            "</n1:testArray>"
            "</n1:test>"
            "</soap:Body>") + xmlEnvEnd
            + '\n'; // added by QXmlStreamWriter::writeEndDocument
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml + expectedRequestBody));

        // Now using persistent headers
        server.resetReceivedBuffers();
        client.setHeader(QLatin1String("header1"), header1);
        client.call(QLatin1String("test"), message, QString::fromLatin1("MySoapAction"));
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml + expectedRequestBody));

        // Now remove the persistent header (using setHeader + empty message)
        server.resetReceivedBuffers();
        client.setHeader(QLatin1String("header1"), KDSoapMessage());
        client.call(QLatin1String("test"), message, QString::fromLatin1("MySoapAction"));
        const QByteArray expectedRequestXmlNoHeader =
            QByteArray(xmlEnvBegin) +
            " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\""
            "><soap:Header/>"; // the empty element does not matter
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXmlNoHeader + expectedRequestBody));
    }

    // Test parsing of complex replies, like with SugarCRM
    void testParseComplexReply()
    {
        HttpServerThread server(complexTypeResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), countryMessageNamespace());
        const KDSoapMessage reply = client.call(QLatin1String("getEmployeeCountry"), countryMessage());
        QVERIFY(!reply.isFault());
        QCOMPARE(reply.arguments().count(), 1);
        const KDSoapValueList lst = reply.arguments().first().childValues();
        QCOMPARE(lst.count(), 3);
        const KDSoapValue id = lst.first();
        QCOMPARE(id.name(), QString::fromLatin1("id"));
        QCOMPARE(id.value().toString(), QString::fromLatin1("12345"));
        const KDSoapValue error = lst.at(1);
        QCOMPARE(error.name(), QString::fromLatin1("error"));
        const KDSoapValueList errorList = error.childValues();
        QCOMPARE(errorList.count(), 3);
        const KDSoapValue number = errorList.at(0);
        QCOMPARE(number.name(), QString::fromLatin1("number"));
        QCOMPARE(number.value().toString(), QString::fromLatin1("0"));
        const KDSoapValue name = errorList.at(1);
        QCOMPARE(name.name(), QString::fromLatin1("name"));
        QCOMPARE(name.value().toString(), QString::fromLatin1("No Error"));
        const KDSoapValue description = errorList.at(2);
        QCOMPARE(description.name(), QString::fromLatin1("description"));
        QCOMPARE(description.value().toString(), QString::fromLatin1("No Error"));
        const KDSoapValue array = lst.at(2);
        QCOMPARE(array.name(), QString::fromLatin1("testArray"));
        //qDebug() << array;
    }

private:
    static QByteArray countryResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body>"
                "<kdab:getEmployeeCountryResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\"><kdab:employeeCountry>France</kdab:employeeCountry></kdab:getEmployeeCountryResponse>"
                " </soap:Body>" + xmlEnvEnd;
    }
    static QByteArray expectedCountryRequest() {
        return QByteArray(xmlEnvBegin) +
                "><soap:Body>"
                "<n1:getEmployeeCountry xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
                "<n1:employeeName>David Ä Faure</n1:employeeName>"
                "</n1:getEmployeeCountry>"
                "</soap:Body>" + xmlEnvEnd;
    }
    static QString countryMessageNamespace() {
        return QString::fromLatin1("http://www.kdab.com/xml/MyWsdl/");
    }
    static KDSoapMessage countryMessage() {
        KDSoapMessage message;
        message.addArgument(QLatin1String("employeeName"), QString::fromUtf8("David Ä Faure"));
        return message;
    }
    void waitForCallFinished(KDSoapPendingCall& pendingCall)
    {
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        QEventLoop loop;
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                &loop, SLOT(quit()));
        loop.exec();

    }

    static QByteArray emptyResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body/>";
    }

    static QByteArray complexTypeResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body xmlns:tns=\"http://www.sugarcrm.com/sugarcrm\">"
                "<ns1:loginResponse xmlns:ns1=\"http://www.sugarcrm.com/sugarcrm\""
                " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">" // useless, but seen in the SoapResponder
                "  <return xsi:type=\"tns:set_entry_result\">"
                "    <id xsi:type=\"xsd:string\">12345</id>"
                "    <error xsi:type=\"tns:error_value\">"
                "       <number xsi:type=\"xsd:string\">0</number>"
                "       <name xsi:type=\"xsd:string\">No Error</name>"
                "       <description xsi:type=\"xsd:string\">No Error</description>"
                "    </error>"
                "    <testArray ns1:attr=\"aValue\" xsi:type=\"soap-enc:Array\" soap-enc:arrayType=\"xsi:string\">"
                "    </testArray>"
                "  </return>"
                "</ns1:loginResponse>"
                "</soap:Body>" + xmlEnvEnd;
    }
};

QTEST_MAIN(BuiltinHttpTest)

#include "builtinhttp.moc"
