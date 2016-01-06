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

#include "wsdl_rpcexample.h"
#include "wsdl_sayhello.h"

#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapServer.h>
#include <KDSoapNamespaceManager.h>
#include <KDSoapPendingCallWatcher.h>
#include <KDSoapPendingCall.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace KDSoapUnitTestHelpers;

class HelloServerObject : public Hello_ServiceServerBase
{
public:
    virtual QString sayHello(const QString &firstName, const QString &lastName)
    {
        m_receivedFirstName = firstName;
        m_receivedLastName = lastName;
        return QString::fromLatin1("You said: ") + firstName + QLatin1Char(' ') + lastName + QLatin1String("!");
    }
    QString receivedFirstName() const
    {
        return m_receivedFirstName;
    }
    QString receivedLastName() const
    {
        return m_receivedLastName;
    }
private:
    QString m_receivedFirstName;
    QString m_receivedLastName;
};

class HelloServer : public KDSoapServer
{
    Q_OBJECT
public:
    HelloServer() : KDSoapServer(), m_lastServerObject(0)
    {
        setPath(QLatin1String("/hello"));
    }
    virtual QObject *createServerObject()
    {
        m_lastServerObject = new HelloServerObject;
        return m_lastServerObject;
    }
    HelloServerObject *lastServerObject()
    {
        return m_lastServerObject;
    }
private:
    HelloServerObject *m_lastServerObject;
};

class RpcExampleServerObject : public RpcExampleServerBase
{
public:
    RpcExampleServerObject()
        : m_heartbeatCalled(false)
    {}
    virtual RPCEXAMPLE__ListKeysResult listKeys(const RPCEXAMPLE__ListKeysParams &params)
    {
        Q_UNUSED(params)
        RPCEXAMPLE__ListKeysResult result;
        result.setKeys(QStringList() << "test1" << "test2" << "test3");
        return result;
    }

    virtual bool pullFile(const RPCEXAMPLE__PullFileParams &params)
    {
        Q_UNUSED(params)
        return false;
    }

    virtual bool putFile(const RPCEXAMPLE__PutFileParams &params)
    {
        Q_UNUSED(params)
        return false;
    }

    virtual QString getFile(const RPCEXAMPLE__GetFileParams &params)
    {
        Q_UNUSED(params)
        return QString();
    }

    virtual RPCEXAMPLE__ExecFileResult execFile(const RPCEXAMPLE__ExecFileParams &params)
    {
        Q_UNUSED(params)
        return RPCEXAMPLE__ExecFileResult();
    }

    virtual RPCEXAMPLE__ListFilesResult listFiles()
    {
        return RPCEXAMPLE__ListFilesResult();
    }

    virtual bool setKey(const RPCEXAMPLE__SetKeyParams &params)
    {
        Q_UNUSED(params)
        return false;
    }

    virtual QString getKey(const RPCEXAMPLE__GetKeyParams &params)
    {
        Q_UNUSED(params)
        return QString();
    }

    virtual bool clearKey(const RPCEXAMPLE__ClearKeyParams &params)
    {
        Q_UNUSED(params)
        return false;
    }

    virtual QString execAction(const RPCEXAMPLE__ExecActionParams &params)
    {
        Q_UNUSED(params)
        return QString();
    }

    virtual void heartbeat(const RPCEXAMPLE__HeartbeatParams &params)
    {
        Q_UNUSED(params);
        m_heartbeatCalled = true;
    }

    virtual void legacyHeartbeat(const RPCEXAMPLE__LegacyHeartbeatParams &params)
    {
        Q_UNUSED(params)
    }

    virtual void message(const RPCEXAMPLE__MessageParams &params)
    {
        Q_UNUSED(params)
    }
    bool heartbeatCalled() const
    {
        return m_heartbeatCalled;
    }
private:
    bool m_heartbeatCalled;
};

class RpcExampleServer : public KDSoapServer
{
    Q_OBJECT
public:
    RpcExampleServer() : KDSoapServer(), m_lastServerObject(0)
    {
        setPath(QLatin1String("/rpcexample"));
    }
    virtual QObject *createServerObject()
    {
        m_lastServerObject = new RpcExampleServerObject;
        return m_lastServerObject;
    }
    RpcExampleServerObject *lastServerObject()
    {
        return m_lastServerObject;
    }
private:
    RpcExampleServerObject *m_lastServerObject;
};

class RPCServerTest : public QObject
{
    Q_OBJECT

private:

    static QByteArray expectedHelloRequest() // http://oreilly.com/catalog/webservess/chapter/ch06.html
    {
        return QByteArray(xmlEnvBegin11()) + ">"
               "<soap:Body>"
               "<n1:sayHello xmlns:n1=\"http://www.ecerami.com/wsdl/HelloService.wsdl\">"
               /*"<n1:sayHello xmlns:n1=\"urn:examples:helloservice\">" // TODO! Add support for * namespace="urn:examples:helloservice" */
               "<firstName xsi:type=\"xsd:string\">Hello</firstName>"
               "<lastName xsi:type=\"xsd:string\">World</lastName>"
               "</n1:sayHello>"
               "</soap:Body>" + xmlEnvEnd()
               + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
    static QByteArray helloResponse()
    {
        return "<?xml version='1.0' encoding='UTF-8'?>"
               "<SOAP-ENV:Envelope "
               "xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
               "xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\" "
               "xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\">"
               "<SOAP-ENV:Body>"
               "<ns1:sayHelloResponse "
               "xmlns:ns1=\"urn:examples:helloservice\" "
               "SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
               "<return xsi:type=\"xsd:string\">You said: Hello World!</return>"
               "</ns1:sayHelloResponse>"
               "</SOAP-ENV:Body>"
               "</SOAP-ENV:Envelope>";
    }

    static QByteArray expectedListKeysRequest()
    {
        return QByteArray(xmlEnvBegin12()) + ">"
               "<soap:Body>"
               "<n1:listKeys xmlns:n1=\"urn:RpcExample\">"
               "<params xsi:type=\"n1:listKeysParams\">"
               "<module xsi:type=\"xsd:string\">Firefox</module>"
               "<base xsi:type=\"xsd:string\"></base>"
               "</params>"
               "</n1:listKeys>"
               "</soap:Body>" + xmlEnvEnd()
               + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
    static QByteArray listKeysResponse()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
               "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:RpcExample=\"urn:RpcExample\">"
               "<SOAP-ENV:Body>"
               "<RpcExample:listKeysResponse SOAP-ENV:encodingStyle=\"http://www.w3.org/2003/05/soap-encoding\">"
               "<result>"
               "<keys>test1</keys>"
               "<keys>test2</keys>"
               "<keys>test3</keys>"
               "</result>"
               "</RpcExample:listKeysResponse>"
               "</SOAP-ENV:Body>"
               "</SOAP-ENV:Envelope>";
    }

private Q_SLOTS:

    void testHello() // http://oreilly.com/catalog/webservess/chapter/ch06.html
    {
        HttpServerThread server(helloResponse(), HttpServerThread::Public);
        Hello_Service service;
        service.setEndPoint(server.endPoint());

        const QString resp = service.sayHello("Hello", "World");
        QCOMPARE(resp, QString::fromLatin1("You said: Hello World!"));

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedHelloRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedHelloRequest().constData()));
        }
    }

    void serverTestHello() // same call, but test both client and server.
    {
        TestServerThread<HelloServer> serverThread;
        HelloServer *server = serverThread.startThread();

        Hello_Service service;
        service.setEndPoint(server->endPoint());

        const QString resp = service.sayHello("Hello", "World");

        QCOMPARE(server->lastServerObject()->receivedFirstName(), QString::fromLatin1("Hello"));
        QCOMPARE(server->lastServerObject()->receivedLastName(), QString::fromLatin1("World"));

        QCOMPARE(resp, QString::fromLatin1("You said: Hello World!"));
    }

    void syncOneWay() // client/server call for a one-way call
    {
        TestServerThread<RpcExampleServer> serverThread;
        RpcExampleServer *server = serverThread.startThread();

        RpcExample service;
        service.setEndPoint(server->endPoint());

        RPCEXAMPLE__HeartbeatParams params;
        service.heartbeat(params);

        QVERIFY(server->lastServerObject()->heartbeatCalled());
    }

    void asyncOneWayCheckNoResponse()
    {
        TestServerThread<RpcExampleServer> serverThread;
        RpcExampleServer *server = serverThread.startThread();

        RpcExample service;
        service.setEndPoint(server->endPoint());

        // emulate service.asyncHeartbeat
        RPCEXAMPLE__HeartbeatParams params;
        KDSoapMessage message;
        message.setUse(KDSoapMessage::EncodedUse);
        KDSoapValue _valueParams(params.serialize(QString::fromLatin1("params")));// /Users/mbroadst/Development/devonit/echosupport/KDSoap/kdwsdl2cpp/src/converter_complextype.cpp:209
        _valueParams.setNamespaceUri(QString::fromLatin1("urn:RpcExample"));
        message.childValues().append(_valueParams);// /Users/mbroadst/Development/devonit/echosupport/KDSoap/kdwsdl2cpp/src/converter_complextype.cpp:223

        QEventLoop eventLoop;
        KDSoapPendingCall pendingCall = service.clientInterface()->asyncCall(QLatin1String("heartbeat"), message);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)), &eventLoop, SLOT(quit()));
        eventLoop.exec();

        KDSoapMessage returnMessage = watcher->returnMessage();
        watcher->deleteLater();

        // same checks as private KDSoapMessage::isNull
        QVERIFY(returnMessage.childValues().isEmpty());
        QVERIFY(returnMessage.childValues().attributes().isEmpty());
        QVERIFY(returnMessage.value().isNull());
    }

    void asyncOneWay() // client/server call for a one-way call, using async methods
    {
        TestServerThread<RpcExampleServer> serverThread;
        RpcExampleServer *server = serverThread.startThread();

        RpcExample service;
        service.setEndPoint(server->endPoint());

        QSignalSpy spy(&service, SIGNAL(heartbeatDone()));
        QVERIFY(spy.isValid());

        qRegisterMetaType<KDSoapMessage>("KDSoapMessage");
        QSignalSpy errorSpy(&service, SIGNAL(heartbeatError(KDSoapMessage)));
        QVERIFY(errorSpy.isValid());

        // Qt5: use spy.wait() instead.
        QEventLoop eventLoop;
        connect(&service, SIGNAL(heartbeatDone()), &eventLoop, SLOT(quit()));
        RPCEXAMPLE__HeartbeatParams params;
        service.asyncHeartbeat(params);
        eventLoop.exec();

        QVERIFY(server->lastServerObject()->heartbeatCalled());
        QCOMPARE(errorSpy.count(), 0);
    }

    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testListKeys()
    {
        HttpServerThread server(listKeysResponse(), HttpServerThread::Public);

        RpcExample service;
        service.setEndPoint(server.endPoint());

        RPCEXAMPLE__ListKeysParams params;
        params.setModule(QString::fromLatin1("Firefox"));
        params.setBase(QString::fromLatin1(""));
        RPCEXAMPLE__ListKeysResult result = service.listKeys(params);

        // Check what we sent
        {
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedListKeysRequest()));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedListKeysRequest().constData()));
        }
        QCOMPARE(result.keys(), QStringList() << QString::fromLatin1("test1") << QString::fromLatin1("test2") << QString::fromLatin1("test3"));
    }

    void serverTestListKeys()
    {
        TestServerThread<RpcExampleServer> serverThread;
        RpcExampleServer *server = serverThread.startThread();

        RpcExample service;
        service.setEndPoint(server->endPoint());

        RPCEXAMPLE__ListKeysParams params;
        params.setModule(QString::fromLatin1("Firefox"));
        params.setBase(QString::fromLatin1(""));
        RPCEXAMPLE__ListKeysResult result = service.listKeys(params);

        QCOMPARE(result.keys(), QStringList() << QString::fromLatin1("test1") << QString::fromLatin1("test2") << QString::fromLatin1("test3"));
    }
};

QTEST_MAIN(RPCServerTest)

#include "test_wsdl_rpc_server.moc"
