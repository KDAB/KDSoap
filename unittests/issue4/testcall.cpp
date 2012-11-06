#include "testcall.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QDebug>

using namespace KDSoapUnitTestHelpers;

TestCall::TestCall()
{
}

static const char* xmlEnvBegin =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<soap:Envelope"
        " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
        " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        " soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"";
static const char* xmlEnvEnd = "</soap:Envelope>";

static QByteArray updateObjsResponse() {
    return QByteArray(xmlEnvBegin) + "><soap:Body>"
            "<updateObjResponseElement xmlns=\"http://test.example.com/types/\">"
            "</updateObjResponseElement>"
            "</soap:Body>" + xmlEnvEnd;
}

void TestCall::test()
{
    HttpServerThread server(updateObjsResponse(), HttpServerThread::Public);
    this->test_client.setEndPoint(server.endPoint());

    TNS0__UpdateObjElement obj_el;
    QList<TNS0__MyObj> objs;
    TNS0__MyObj t;
    t.setField1("test1");
    t.setId("1");
    objs.append(t);
    TNS0__Credential credential;
    credential.setId("id");
    credential.setPassWord("password");
    obj_el.setArrayOfObj(objs);
    obj_el.setCredential(credential);
    this->test_client.updateObjs(obj_el);

    const QByteArray expectedData = QByteArray(xmlEnvBegin) + "><soap:Body>"
    "<n1:updateObjElement xmlns:n1=\"http://test.example.com/types/\">"
     "<n1:arrayOfObj>"
       "<n1:Id>1</n1:Id>"
       "<n1:Field1>test1</n1:Field1>"
     "</n1:arrayOfObj>"
     "<n1:credential>"
       "<n1:Id>id</n1:Id>"
       "<n1:PassWord>password</n1:PassWord>"
     "</n1:credential>"
    "</n1:updateObjElement></soap:Body></soap:Envelope>";

    QVERIFY(xmlBufferCompare(server.receivedData(), expectedData));
}

QTEST_MAIN(TestCall)
