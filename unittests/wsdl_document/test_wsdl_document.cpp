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

#include "wsdl_mywsdl_document.h"
#include "wsdl_thomas-bayer.h"
#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>
#include <KDSoapClientInterface.h>
#include <KDSoapMessage.h>
#include <KDSoapPendingCallWatcher.h>
#include <KDSoapNamespaceManager.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#ifndef QT_NO_OPENSSL
#include <KDSoapSslHandler.h>
#include <QSslSocket>
#endif

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

static const char* xmlBegin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
static const char* xmlNamespaces = "xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
    " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
    " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
    " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
static const char* myWsdlNamespace = "http://www.kdab.com/xml/MyWsdl/";

class WsdlDocumentTest : public QObject
{
    Q_OBJECT

private:
    static KDAB__AddEmployee addEmployeeParameters()
    {
        KDAB__EmployeeAchievements achievements;
        QList<KDAB__EmployeeAchievement> lst;
        KDAB__EmployeeAchievement achievement1;
        achievement1.setType(QByteArray("Project"));
        achievement1.setLabel(QString::fromLatin1("Management"));
        achievement1.setTime(QDate(2011, 06, 27));
        lst.append(achievement1);
        KDAB__EmployeeAchievement achievement2;
        achievement2.setType(QByteArray("Development"));
        achievement2.setLabel(QString::fromLatin1("C++"));
        achievement2.setTime(QString::fromLatin1("today"));
        lst.append(achievement2);
        achievements.setItems(lst);
        KDAB__EmployeeType employeeType;
        employeeType.setType(KDAB__EmployeeTypeEnum::Developer);
        employeeType.setOtherRoles(QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader);
        employeeType.setTeam(QString::fromLatin1("Minitel"));

        KDAB__AddEmployee addEmployeeParams;
        addEmployeeParams.setEmployeeType(employeeType);
        addEmployeeParams.setEmployeeName(QString::fromLatin1("David Faure"));
        addEmployeeParams.setEmployeeCountry(QString::fromLatin1("France"));
        addEmployeeParams.setEmployeeAchievements(achievements);
        KDAB__EmployeeId id;
        id.setId(5);
        addEmployeeParams.setEmployeeId(id);
        return addEmployeeParams;
    }

    static QByteArray requestXmlTemplate()
    {
        return QByteArray(xmlEnvBegin) +
                " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">%1"
                "<soap:Body>"
                "<n1:addEmployee>"
                "<n1:employeeType n1:type=\"Developer\">"
                "<n1:team>Minitel</n1:team>"
                "<n1:otherRoles>TeamLeader</n1:otherRoles>"
                "</n1:employeeType>"
                "<n1:employeeName>David Faure</n1:employeeName>"
                "<n1:employeeCountry>France</n1:employeeCountry>"
                "<n1:employeeAchievements>"
                "<n1:item>"
                "<n1:type>50726f6a656374</n1:type>" // Project
                "<n1:label>Management</n1:label>"
                "<n1:time>2011-06-27</n1:time>"
                "</n1:item>"
                "<n1:item>"
                "<n1:type>446576656c6f706d656e74</n1:type>" // Development
                "<n1:label>C++</n1:label>"
                "<n1:time>today</n1:time>"
                "</n1:item>"
                "</n1:employeeAchievements>"
                "<n1:employeeId>"
                "<n1:id>5</n1:id>"
                "</n1:employeeId>"
                "</n1:addEmployee>"
                "</soap:Body>" + xmlEnvEnd
                + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
    static QByteArray expectedHeader() {
        return QByteArray("<soap:Header>"
                          "<n1:LoginElement>"
                          "<n1:user>foo</n1:user>"
                          "<n1:pass>bar</n1:pass>"
                          "</n1:LoginElement>"
                          "<n1:SessionElement>"
                          "<n1:sessionId>id</n1:sessionId>"
                          "</n1:SessionElement>"
                          "</soap:Header>");
    }
    static QByteArray addEmployeeResponse() {
        return QByteArray(xmlEnvBegin) + ">"
        "<soap:Header xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">"
        "<kdab:SessionElement sessionId=\"returned_id\"/>"
        "</soap:Header>"
        "<soap:Body>"
        "<kdab:addEmployeeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">466F6F</kdab:addEmployeeResponse>" // "Foo"
        " </soap:Body>" + xmlEnvEnd;
    }

private Q_SLOTS:

    void initTestCase()
    {
        qRegisterMetaType<KDSoapMessage>();
#ifndef QT_NO_OPENSSL
        qRegisterMetaType< QList<QSslError> >();
        qRegisterMetaType<KDSoapSslHandler *>();
#endif
    }

    // Using wsdl-generated code, make a call, and check the xml that was sent,
    // and check that the server's response was correctly parsed.
    void testMyWsdlPublic()
    {
        HttpServerThread server(addEmployeeResponse(), HttpServerThread::Public);

        // For testing the http server with telnet or wget:
        //httpGet(server.endPoint());
        //QEventLoop testLoop;
        //testLoop.exec();

        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__LoginElement login;
        login.setUser(QLatin1String("foo"));
        login.setPass(QLatin1String("bar"));
        KDAB__SessionElement session;
        session.setSessionId(QLatin1String("id"));

        service.setLoginHeader(login);
        service.setSessionHeader(session);

        KDAB__AddEmployee addEmployeeParams = addEmployeeParameters();

        QByteArray ret = service.addEmployee(addEmployeeParams);
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QByteArray("Foo"));
        // Check what we sent
        {
            QByteArray expectedRequestXml = requestXmlTemplate();
            expectedRequestXml.replace("%1", expectedHeader());
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
            QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedRequestXml.constData()));
            QVERIFY(server.receivedHeaders().contains("SoapAction: \"http://www.kdab.com/AddEmployee\""));
        }

        // Test utf8
        addEmployeeParams.setEmployeeName(QString::fromUtf8("Hervé"));
        addEmployeeParams.setEmployeeCountry(QString::fromUtf8("фгн7")); // random russian letters
        {
            // This second call also tests that persistent headers are indeed persistent.
            server.resetReceivedBuffers();
            QByteArray expectedRequestXml = requestXmlTemplate();
            expectedRequestXml.replace("%1", expectedHeader());
            expectedRequestXml.replace("David Faure", "Hervé");
            expectedRequestXml.replace("France", "фгн7");
            ret = service.addEmployee(addEmployeeParams);
            QVERIFY(service.lastError().isEmpty());
            QCOMPARE(ret, QByteArray("Foo"));
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        }

        // Test removing headers
        {
            server.resetReceivedBuffers();
            service.clearLoginHeader();
            service.clearSessionHeader();
            ret = service.addEmployee(addEmployeeParameters());
            QByteArray expectedRequestXml = requestXmlTemplate();
            expectedRequestXml.replace("%1", "<soap:Header/>");
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        }
    }

#ifndef QT_NO_OPENSSL
    void testSslError()
    {
        if (!QSslSocket::supportsSsl())
            return; // see above
        // Use SSL on the server, without adding the CA certificate (done by setSslConfiguration())
        HttpServerThread server(addEmployeeResponse(), HttpServerThread::Ssl);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());
        QVERIFY(server.endPoint().startsWith(QLatin1String("https")));
        QSignalSpy sslErrorsSpy(service.clientInterface()->sslHandler(), SIGNAL(sslErrors(KDSoapSslHandler*, QList<QSslError>)));
        // We need to use async API to test sslHandler, see documentation there.
        ListEmployeesJob *job = new ListEmployeesJob(&service);
        connect(job, SIGNAL(finished(KDSoapJob*)), this, SLOT(slotListEmployeesJobFinished(KDSoapJob*)));
        job->start();
        m_eventLoop.exec();
        // Disable SSL so that termination can happen normally (do it asap, in case of failure below)
        server.disableSsl();

        QVERIFY2(job->faultAsString().contains(QLatin1String("SSL handshake failed")), qPrintable(service.lastError()));
        QCOMPARE(sslErrorsSpy.count(), 1);
#ifdef Q_OS_UNIX // Windows seems to get "Unknown error". Bah...
        const QList<QSslError> errors = sslErrorsSpy.at(0).at(1).value<QList<QSslError> >();
        QCOMPARE((int)errors.at(0).error(), (int)QSslError::UnableToGetLocalIssuerCertificate);
        QCOMPARE((int)errors.at(1).error(), (int)QSslError::CertificateUntrusted);
        QCOMPARE((int)errors.at(2).error(), (int)QSslError::UnableToVerifyFirstCertificate);
#endif
    }

    void testSslErrorHandled()
    {
        if (!QSslSocket::supportsSsl())
            return; // see above
        // Use SSL on the server, without adding the CA certificate (done by setSslConfiguration())
        HttpServerThread server(addEmployeeResponse(), HttpServerThread::Ssl);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());
        connect(service.clientInterface()->sslHandler(), SIGNAL(sslErrors(KDSoapSslHandler*, QList<QSslError>)),
                this, SLOT(slotSslHandlerErrors(KDSoapSslHandler*, QList<QSslError>)));
        AddEmployeeJob *job = new AddEmployeeJob(&service);
        job->setParameters(addEmployeeParameters());
        connect(job, SIGNAL(finished(KDSoapJob*)), this, SLOT(slotAddEmployeeJobFinished(KDSoapJob*)));
        job->start();
        m_eventLoop.exec();
        // Disable SSL so that termination can happen normally (do it asap, in case of failure below)
        server.disableSsl();
        QCOMPARE(m_errors.count(), 3);
        QCOMPARE(job->faultAsString(), QString());
        QCOMPARE(QString::fromLatin1(job->resultParameters().constData()), QString::fromLatin1("Foo"));
    }


    void testMyWsdlSSL()
    {
        if (!QSslSocket::supportsSsl()) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
            QSKIP("No SSL support on this machine, check that ssleay.so/ssleay32.dll is installed");
#else
            QSKIP("No SSL support on this machine, check that ssleay.so/ssleay32.dll is installed", SkipAll);
#endif
        }

#ifndef QT_NO_SSLSOCKET
        QVERIFY(KDSoapUnitTestHelpers::setSslConfiguration());
#endif

        HttpServerThread server(addEmployeeResponse(), HttpServerThread::Ssl);

        // For testing the http server with telnet or wget:
        //qDebug() << "endPoint=" << server.endPoint();
        //httpGet(server.endPoint());
        //QEventLoop testLoop;
        //testLoop.exec();

        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());
        QVERIFY(server.endPoint().startsWith(QLatin1String("https")));

        KDAB__LoginElement login;
        login.setUser(QLatin1String("foo"));
        login.setPass(QLatin1String("bar"));
        KDAB__SessionElement session;
        session.setSessionId(QLatin1String("id"));

        service.setLoginHeader(login);
        service.setSessionHeader(session);

        QByteArray ret = service.addEmployee(addEmployeeParameters());
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();

        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(ret, QByteArray("Foo"));
        // Check what we sent
        QByteArray expectedRequestXml = requestXmlTemplate();
        expectedRequestXml.replace("%1", expectedHeader());
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
        QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedRequestXml.constData()));
        QVERIFY(server.receivedHeaders().contains("SoapAction: \"http://www.kdab.com/AddEmployee\""));
    }

#endif

    void testSimpleType()
    {
        HttpServerThread server(countryResponse(), HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__EmployeeNameParams params;
        params.setEmployeeName(KDAB__EmployeeName(QString::fromUtf8("David Ä Faure")));
        const KDAB__EmployeeCountryResponse employeeCountryResponse = service.getEmployeeCountry(params);
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(employeeCountryResponse.employeeCountry().value(), QString::fromLatin1("France"));
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedCountryRequest()));
        QCOMPARE(QString::fromUtf8(server.receivedData().constData()), QString::fromUtf8(expectedCountryRequest().constData()));
    }

    void testEmptyResponse()
    {
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());
        KDAB__EmployeeNameParams params;
        const KDAB__EmployeeCountryResponse employeeCountryResponse = service.getEmployeeCountry(params);
        QCOMPARE(service.lastError(), QString()); // no error, just an empty struct
        QCOMPARE(employeeCountryResponse.employeeCountry().value(), QString());
    }

    // Test enum deserialization
    void testEnums()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<kdab:getEmployeeTypeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\" kdab:type=\"Developer\">"
                                    "<kdab:team>Minitel</kdab:team>"
                                    "<kdab:otherRoles>TeamLeader</kdab:otherRoles>"
                                  "</kdab:getEmployeeTypeResponse>"
                                  "</soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__EmployeeNameParams params;
        params.setEmployeeName(KDAB__EmployeeName(QLatin1String("Joe")));
        const KDAB__EmployeeType employeeType = service.getEmployeeType(params);
        if (!service.lastError().isEmpty())
            qDebug() << service.lastError();
        QVERIFY(service.lastError().isEmpty());
        QCOMPARE(employeeType.team().value().value(), QLatin1String("Minitel"));
        QCOMPARE(employeeType.otherRoles().count(), 1);
        QCOMPARE(employeeType.otherRoles().at(0).type(), KDAB__EmployeeTypeEnum::TeamLeader);
        QCOMPARE((int)employeeType.type().type(), (int)KDAB__EmployeeTypeEnum::Developer);
    }

    void testSoapVersion()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<kdab:getEmployeeTypeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\" kdab:type=\"Developer\">"
                                    "<kdab:team>Minitel</kdab:team>"
                                    "<kdab:otherRoles>TeamLeader</kdab:otherRoles>"
                                  "</kdab:getEmployeeTypeResponse>"
                                  "</soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__EmployeeNameParams params;
        params.setEmployeeName(KDAB__EmployeeName(QLatin1String("Joe")));

        service.setSoapVersion(KDSoapClientInterface::SOAP1_1);
        KDAB__EmployeeType employeeType = service.getEmployeeType(params);
        QVERIFY(service.lastError().isEmpty());

        service.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        KDAB__EmployeeType employeeType2 = service.getEmployeeType(params);
        QVERIFY(service.lastError().isEmpty());
    }

    // Was http://www.service-repository.com/service/wsdl?id=163859, but it disappeared.
    // Local WSDL file: thomas-bayer.wsdl
    void testSequenceInResponse()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<getCountriesResponse><country>Great Britain</country><country>Ireland</country></getCountriesResponse>"
                                  " </soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);

        NamesServiceService serv;
        serv.setEndPoint(server.endPoint());
        const QStringList countries = serv.getCountries().country(); // the wsdl should have named it "countries"...
        QCOMPARE(countries.count(), 2);
        QCOMPARE(countries[0], QString::fromLatin1("Great Britain"));
        QCOMPARE(countries[1], QString::fromLatin1("Ireland"));

        const QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) + ">"
            "<soap:Body>"
            "<n1:getCountries xmlns:n1=\"http://namesservice.thomas_bayer.com/\" xsi:nil=\"true\"/>"
            "</soap:Body>" + xmlEnvEnd;
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        // Same test without using generated code
        {
            const QString messageNamespace = QString::fromLatin1("http://namesservice.thomas_bayer.com/");
            KDSoapClientInterface client(server.endPoint(), messageNamespace);
            KDSoapMessage message;
            KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("getCountries"), message);
            KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
            connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                    this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
            m_eventLoop.exec();
            //qDebug() << m_returnMessage;

            QCOMPARE(m_returnMessage.arguments()[0].value().toString(), QString::fromLatin1("Great Britain"));
            QCOMPARE(m_returnMessage.arguments()[1].value().toString(), QString::fromLatin1("Ireland"));
        }
    }

    void testAnyType()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                                  "<kdab:AnyTypeResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">"
                                  "<xsd:schema><xsd:test>response</xsd:test></xsd:schema>"
                                  "<kdab:return xsd:type=\"xsd:int\">42</kdab:return>"
                                  "<kdab:return xsd:type=\"xsd:string\">Forty-two</kdab:return>"
                                  "<kdab:return xsd:type=\"kdab:TeamName\">Minitel</kdab:return>"
                                  "<kdab:return xsd:type=\"kdab:EmployeeAchievement\">"
                                    "<kdab:type>Project</kdab:type>"
                                    "<kdab:label>Management</kdab:label>"
                                  "</kdab:return>"
                                  "</kdab:AnyTypeResponse>"
                                  "</soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        KDAB__AnyType anyType;
        anyType.setInput(KDSoapValue(QString::fromLatin1("foo"), QString::fromLatin1("Value"), KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("string")));
        KDSoapValueList schemaChildValues;
        KDSoapValue testVal(QLatin1String("test"), QString::fromLatin1("input"));
        testVal.setNamespaceUri(KDSoapNamespaceManager::xmlSchema2001());
        schemaChildValues.append(testVal);
        KDSoapValue inputSchema(QLatin1String("schema"), schemaChildValues);
        inputSchema.setNamespaceUri(KDSoapNamespaceManager::xmlSchema2001());
        anyType.setSchema(inputSchema);
        const KDAB__AnyTypeResponse response = service.testAnyType(anyType);
        const QList<KDSoapValue> values = response._return();
        QCOMPARE(values.count(), 4);
        QCOMPARE(values.at(0).value().toInt(), 42);
        QCOMPARE(values.at(1).value().toString(), QString::fromLatin1("Forty-two"));
        QCOMPARE(values.at(2).value().toString(), QString::fromLatin1("Minitel"));
        const QList<KDSoapValue> achievements = values.at(3).childValues();
        QCOMPARE(achievements.count(), 2);
        QCOMPARE(achievements.at(0).value().toString(), QString::fromLatin1("Project"));
        QCOMPARE(achievements.at(1).value().toString(), QString::fromLatin1("Management"));

        const QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) + ">"
            "<soap:Body>"
            "<n1:AnyType xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
            "<xsd:schema><xsd:test>input</xsd:test></xsd:schema>"
            "<foo>Value</foo>"
            "</n1:AnyType>"
            "</soap:Body>" + xmlEnvEnd;
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));

        const KDSoapValue schema = response.schema();
        QCOMPARE(schema.name(), QString::fromLatin1("schema"));
        QCOMPARE(schema.namespaceUri(), KDSoapNamespaceManager::xmlSchema2001());
        const QByteArray expectedResponseSchemaXml =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<xsd:schema"
                " xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\""
                " xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\""
                " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                ">"
                "<xsd:test>response</xsd:test>"
                "</xsd:schema>";
        QVERIFY(xmlBufferCompare(schema.toXml(), expectedResponseSchemaXml));
    }

    // Document/literal, not wrapped -> the operation name doesn't appear
    void testByteArrays()
    {
        // Prepare response
        QByteArray responseData = QByteArray(xmlEnvBegin) + "><soap:Body>"
                "<kdab:Telegram xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\">466f6f</kdab:Telegram>"
                "</soap:Body>" + xmlEnvEnd;
        HttpServerThread server(responseData, HttpServerThread::Public);
        MyWsdlDocument service;
        service.setEndPoint(server.endPoint());

        const KDAB__TelegramType ret = service.sendRawTelegram(KDAB__TelegramType("Hello"));
        QCOMPARE(service.lastError(), QString());
        QCOMPARE(ret.value(), QByteArray("Foo"));

        const QByteArray expectedRequestXml =
            QByteArray(xmlEnvBegin) + ">"
            "<soap:Body>"
            "<n1:Telegram xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">48656c6c6f</n1:Telegram>"
            "</soap:Body>" + xmlEnvEnd;
            QVERIFY(xmlBufferCompare(server.receivedData(), expectedRequestXml));
    }

    // Client+server tests

    void testServerAddEmployee();
    void testServerAddEmployeeJob();
    void testServerPostByHand();
    void testServerEmptyArgs();
    void testServerFault();
    void testSendTelegram();
    void testSendHugeTelegram();
    void testServerDelayedCall();
    void testSyncCallAfterServerDelayedCall();
    void testServerTwoDelayedCalls();
    void testServerDifferentPath();

public slots:
    void slotFinished(KDSoapPendingCallWatcher* watcher)
    {
        m_returnMessage = watcher->returnMessage();
        m_eventLoop.quit();
    }

    void slotDelayedAddEmployeeDone(const QByteArray& data)
    {
        //qDebug() << Q_FUNC_INFO << data;
        m_delayedData << data;
        if (--m_expectedDelayedCalls == 0) {
            m_eventLoop.quit();
        }
    }
    void slotAddEmployeeJobFinished(KDSoapJob*)
    {
        // TODO: we should emit a signal with the proper job type?
        //AddEmployeeJob* addJob = static_cast<AddEmployeeJob *>(job);
        m_eventLoop.quit();
    }
    void slotListEmployeesJobFinished(KDSoapJob*)
    {
        m_eventLoop.quit();
    }

#ifndef QT_NO_OPENSSL
    void slotSslHandlerErrors(KDSoapSslHandler* handler, const QList<QSslError>& errors)
    {
        // This is just for testing error handling. Don't write this in your actual code...
#ifdef Q_OS_UNIX // Windows seems to get "Unknown error". Bah...
        if (errors.at(0).error() == QSslError::UnableToGetLocalIssuerCertificate)
#endif
            handler->ignoreSslErrors();
        m_errors = errors;
    }
#endif

private:
    QEventLoop m_eventLoop;
    KDSoapMessage m_returnMessage;
#ifndef QT_NO_OPENSSL
    QList<QSslError> m_errors;
#endif

    int m_expectedDelayedCalls;
    QList<QByteArray> m_delayedData;

    static QByteArray emptyResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body/>" + xmlEnvEnd;
    }
    static QByteArray countryResponse() {
        return QByteArray(xmlEnvBegin) + "><soap:Body>"
                "<kdab:getEmployeeCountryResponse xmlns:kdab=\"http://www.kdab.com/xml/MyWsdl/\"><kdab:employeeCountry>France</kdab:employeeCountry></kdab:getEmployeeCountryResponse>"
                " </soap:Body>" + xmlEnvEnd;
    }
    static QByteArray expectedCountryRequest() {
        return QByteArray(xmlEnvBegin) +
                "><soap:Body>"
                "<n1:EmployeeNameParams xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
                "<n1:employeeName>"
                "David Ä Faure"
                "</n1:employeeName>"
                "</n1:EmployeeNameParams>"
                "</soap:Body>" + xmlEnvEnd
                + '\n'; // added by QXmlStreamWriter::writeEndDocument
    }
};

#include "KDSoapServerObjectInterface.h"
#include "KDSoapServer.h"

class MyJob : public QObject
{
    Q_OBJECT
public:
    MyJob(const KDSoapDelayedResponseHandle& handle)
        : m_handle(handle)
    {
        QTimer::singleShot(20, this, SLOT(slotDone()));
    }
    KDSoapDelayedResponseHandle responseHandle() const { return m_handle; }
Q_SIGNALS:
    void done(MyJob*);
private Q_SLOTS:
    void slotDone() {
        emit done(this);
    }
private:
    KDSoapDelayedResponseHandle m_handle;
};

class NameServiceServerObject : public NamesServiceServiceServerBase /* generated from thomas-bayer.wsdl */
{
    Q_OBJECT
public:
    virtual TNS__GetCountriesResponse getCountries( const TNS__GetCountries& ) {
        TNS__GetCountriesResponse response;
        response.setCountry(QStringList() << QLatin1String("Great Britain") << QLatin1String("Ireland"));
        return response;
    }
      TNS__GetNamesInCountryResponse getNamesInCountry( const TNS__GetNamesInCountry& parameters ) {
          TNS__GetNamesInCountryResponse response;
          if (parameters.country() == QLatin1String("Ireland")) {
              response.setName(QStringList() << QLatin1String("Name1") << QLatin1String("Name2"));
          }
          return response;
      }

      void getNameInfoResponse( const KDSoapDelayedResponseHandle& responseHandle, const TNS__GetNameInfoResponse& ret );
      virtual TNS__GetNameInfoResponse getNameInfo( const TNS__GetNameInfo& parameters ) {
          setFault(QLatin1String("Server.Implementation"), QLatin1String("Not implemented"), QLatin1String(metaObject()->className()), parameters.name());
          return TNS__GetNameInfoResponse();
      }
};

class DocServerObject : public MyWsdlDocumentServerBase /* generated from mywsdl_document.wsdl */
{
    Q_OBJECT
public:
    QByteArray addEmployee( const KDAB__AddEmployee& parameters ) {
        //qDebug() << "addEmployee called";
        const QString name = KDAB__LimitedString(parameters.employeeName()).value();
        if (name.isEmpty()) {
            setFault(QLatin1String("Client.Data"), QLatin1String("Empty employee name"),
                     QLatin1String("DocServerObject"), tr("Employee name must not be empty"));
            return QByteArray();
        }
        // TODO generate a helper method for this!
        KDSoapHeaders headers;
        KDSoapMessage sessionHeader;
        KDSoapValue sessionElement(QString::fromLatin1("SessionElement"), QVariant());
        sessionElement.setNamespaceUri(QLatin1String(myWsdlNamespace));
        KDSoapValue sessionIdElement(QString::fromLatin1("sessionId"), QString::fromLatin1("returned_id"));
        sessionElement.childValues().append(sessionIdElement);
        sessionHeader.childValues().append(sessionElement);
        headers.append(sessionHeader);
        setResponseHeaders(headers);
        return "added " + name.toLatin1();
    }

    QByteArray delayedAddEmployee( const KDAB__AddEmployee& parameters ) {
        //qDebug() << "delayedAddEmployee called";
        Q_UNUSED(parameters);

        KDSoapDelayedResponseHandle handle = prepareDelayedResponse();
        MyJob* job = new MyJob(handle);
        connect(job, SIGNAL(done(MyJob*)), this, SLOT(slotDelayedResponse(MyJob*)));
        return "THIS VALUE IS IGNORED";
    }

    void listEmployees() {
        m_lastMethodCalled = QLatin1String("listEmployees");
    }
    void heartbeat() {
        m_lastMethodCalled = QLatin1String("heartbeat");
    }

    KDAB__AnyTypeResponse testAnyType( const KDAB__AnyType& parameters ) {
        KDAB__AnyTypeResponse response;
        response.setReturn(QList<KDSoapValue>() << parameters.input());
        return response;
    }

    KDAB__EmployeeCountryResponse getEmployeeCountry( const KDAB__EmployeeNameParams& employeeNameParams ) {
        KDAB__EmployeeCountryResponse resp;
        if (QString(employeeNameParams.employeeName().value()) == QLatin1String("David")) {
            resp.setEmployeeCountry(QString::fromLatin1("France"));
        } else {
            resp.setEmployeeCountry(QString::fromLatin1("Unknown country!"));
        }
        return resp;
    }

    KDAB__EmployeeType getEmployeeType( const KDAB__EmployeeNameParams& employeeNameParams ) {
        KDAB__EmployeeType type;
        if (QString(employeeNameParams.employeeName().value()) == QLatin1String("David")) {
            type.setTeam(KDAB__TeamName(QString::fromLatin1("Minitel")));
        }
        return type;
    }

    KDAB__TelegramType sendRawTelegram( const KDAB__TelegramType& telegram ) {
        return QByteArray("Got ") + telegram;
    }

    KDAB__TelegramResponse sendTelegram( const KDAB__TelegramRequest& parameters ) {
        KDAB__TelegramResponse resp;
        resp.setTelegramHex(QByteArray("Received ") + parameters.telegramHex());
        resp.setTelegramBase64(QByteArray("Received ") + parameters.telegramBase64());
        return resp;
    }

    // Normally you don't reimplement this. This is just to store req and resp for the unittest.
    void processRequest( const KDSoapMessage &request, KDSoapMessage &response, const QByteArray& soapAction ) {
        m_request = request;
        MyWsdlDocumentServerBase::processRequest(request, response, soapAction);
        m_response = response;
        //qDebug() << "processRequest: done. " << this << "Response name=" << response.name();
    }

    void processRequestWithPath(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray &soapAction, const QString &path) {
        Q_UNUSED(request);
        Q_UNUSED(response);
        if (path == QLatin1String("/xml/NamesService?foo")
            && soapAction == "http://namesservice.thomas_bayer.com/getCountries") {
            m_nameServiceServerObject.processRequest(request, response, soapAction);
        } else {
            setFault(QLatin1String("Client.Data"),
                     QString::fromLatin1("Action %1 not found in path %2").arg(QLatin1String(soapAction.constData()), path));
        }
    }

    KDSoapMessage m_request;
    KDSoapMessage m_response;
    QString m_lastMethodCalled;

private Q_SLOTS:
    void slotDelayedResponse(MyJob* job)
    {
        delayedAddEmployeeResponse(job->responseHandle(), QByteArray("delayed reply works"));
        job->deleteLater();
        // TODO test delayed fault.
    }
private:
    NameServiceServerObject m_nameServiceServerObject;
};

class DocServer : public KDSoapServer
{
    Q_OBJECT
public:
    DocServer() : KDSoapServer(), m_lastServerObject(0) {
        setPath(QLatin1String("/xml"));
    }
    virtual QObject* createServerObject() { m_lastServerObject = new DocServerObject; return m_lastServerObject; }

    DocServerObject* lastServerObject() { return m_lastServerObject; }

private:
    DocServerObject* m_lastServerObject; // only for unittest purposes
};

void WsdlDocumentTest::testServerAddEmployee()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();

    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());
    QByteArray ret = service.addEmployee(addEmployeeParameters());
    QVERIFY(server->lastServerObject());
    const QByteArray expectedResponseXml =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<n1:addEmployeeMyResponse xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
                "6164646564204461766964204661757265"
                "</n1:addEmployeeMyResponse>\n";
    //qDebug() << server->lastServerObject() << "response name" << server->lastServerObject()->m_response.name();
    // Note: that's the response as sent by the generated code.
    // But then the server socket code will call messageToXml, possibly with a method name,
    // we can't debug that here. The next test is for that.
    QVERIFY(xmlBufferCompare(server->lastServerObject()->m_response.toXml(), expectedResponseXml));
    QCOMPARE(service.lastError(), QString());
    QCOMPARE(QString::fromLatin1(ret.constData()), QString::fromLatin1("added David Faure"));
    KDSoapMessage sessionHeader = service.clientInterface()->lastResponseHeaders().header(QLatin1String("SessionElement"));
    KDSoapValue sessionIdValue = sessionHeader.arguments().child("sessionId");
    QCOMPARE(sessionIdValue.value().toString(), QLatin1String("returned_id"));
}

void WsdlDocumentTest::testServerAddEmployeeJob()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();

    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());
    AddEmployeeJob* job = new AddEmployeeJob(&service);
    job->setParameters(addEmployeeParameters());
    job->start();
    connect(job, SIGNAL(finished(KDSoapJob*)), this, SLOT(slotAddEmployeeJobFinished(KDSoapJob*)));
    m_eventLoop.exec();

    QCOMPARE(service.lastError(), QString());
    QByteArray ret = job->resultParameters();
    QCOMPARE(QString::fromLatin1(ret.constData()), QString::fromLatin1("added David Faure"));
    KDAB__SessionElement outputSession = job->outputHeader();
    QCOMPARE(outputSession.sessionId(), QLatin1String("returned_id"));
}

static QByteArray rawCountryMessage() {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><soap:Body><n1:getEmployeeCountry xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
    "<employeeName>David</employeeName>"
    "</n1:getEmployeeCountry></soap:Body></soap:Envelope>";
}

void WsdlDocumentTest::testServerPostByHand()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();

    QUrl url(server->endPoint());
    QNetworkRequest request(url);
    request.setRawHeader("SoapAction", "http://www.kdab.com/xml/MyWsdl/getEmployeeCountry");
    QString soapHeader = QString::fromLatin1("text/xml;charset=utf-8");
    request.setHeader(QNetworkRequest::ContentTypeHeader, soapHeader.toUtf8());
    QNetworkAccessManager accessManager;
    QNetworkReply* reply = accessManager.post(request, rawCountryMessage());
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    const QByteArray response = reply->readAll();
    const QByteArray expected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "<soap:Body>"
    "<n1:EmployeeCountryResponse xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
      "<n1:employeeCountry>France</n1:employeeCountry>"
    "</n1:EmployeeCountryResponse>"
    "</soap:Body></soap:Envelope>\n";
    QVERIFY(xmlBufferCompare(response, expected));
    QCOMPARE(response.constData(), expected.constData());
}

void WsdlDocumentTest::testServerEmptyArgs()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();

    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());
    service.heartbeat(); // ensure the method is generated
    service.listEmployees();
    QVERIFY(server->lastServerObject());
    QCOMPARE(server->lastServerObject()->m_lastMethodCalled, QString::fromLatin1("listEmployees"));
    QCOMPARE(service.lastError(), QString());
}

void WsdlDocumentTest::testServerFault() // test the error signals emitted on error, in async calls
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();

    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());
    QSignalSpy addEmployeeErrorSpy(&service, SIGNAL(addEmployeeError(KDSoapMessage)));
    QSignalSpy soapErrorSpy(&service, SIGNAL(soapError(QString, KDSoapMessage)));
    service.asyncAddEmployee(KDAB__AddEmployee());

    connect(&service, SIGNAL(soapError(QString, KDSoapMessage)), &m_eventLoop, SLOT(quit()));
    m_eventLoop.exec();

    QCOMPARE(soapErrorSpy.count(), 1);
    QCOMPARE(addEmployeeErrorSpy.count(), 1);
    QCOMPARE(soapErrorSpy[0][0].toString(), QString::fromLatin1("addEmployee"));
    KDSoapMessage msg = soapErrorSpy[0][1].value<KDSoapMessage>();
    QCOMPARE(msg.faultAsString(), QString::fromLatin1("Fault code Client.Data: Empty employee name (DocServerObject)"));
}

void WsdlDocumentTest::testSendTelegram()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();

    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());

    KDAB__TelegramRequest req;
    req.setTelegramHex(KDAB__TelegramType("Hello"));
    req.setTelegramBase64(QByteArray("Hello"));
    const KDAB__TelegramResponse ret = service.sendTelegram(req);
    QCOMPARE(service.lastError(), QString());
    QCOMPARE(ret.telegramHex().value(), QByteArray("Received Hello"));
    QCOMPARE(ret.telegramBase64().constData(), "Received Hello");

    // Check the request as received by the server.
    const QByteArray expectedRequestXml =
        QByteArray(xmlBegin) + "<TelegramRequest " + xmlNamespaces + ">"
           "<TelegramHex>48656c6c6f</TelegramHex>"
           "<TelegramBase64>SGVsbG8=</TelegramBase64>"
          "</TelegramRequest>";
    const QString msgNS = QString::fromLatin1("http://www.kdab.com/xml/MyWsdl/");
    // Note that "qualified" is false in m_request, since it was created dynamically by the server -> no namespaces
    QVERIFY(xmlBufferCompare(server->lastServerObject()->m_request.toXml(KDSoapValue::LiteralUse, msgNS), expectedRequestXml));

    const QByteArray expectedResponseXml =
        QByteArray(xmlBegin) + "<n1:TelegramResponse " + xmlNamespaces + " xmlns:n1=\"http://www.kdab.com/xml/MyWsdl/\">"
            "<n1:TelegramHex>52656365697665642048656c6c6f</n1:TelegramHex>"
            "<n1:TelegramBase64>UmVjZWl2ZWQgSGVsbG8=</n1:TelegramBase64>"
          "</n1:TelegramResponse>";
    // m_response, however, has qualified = true (set by the generated code).
    QVERIFY(xmlBufferCompare(server->lastServerObject()->m_response.toXml(KDSoapValue::LiteralUse, msgNS), expectedResponseXml));
}

void WsdlDocumentTest::testSendHugeTelegram()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();

    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());

    QByteArray hugePayload;
    hugePayload.resize(50000);
    char ch = 'A';
    for (int i = 0 ; i < hugePayload.size() ; ++i) {
        hugePayload[i] = ++ch;
        if (ch == 'z' + 1)
            ch = 'A';
    }

    KDAB__TelegramRequest req;
    req.setTelegramHex(KDAB__TelegramType(hugePayload));
    req.setTelegramBase64(QByteArray(hugePayload));
    const KDAB__TelegramResponse ret = service.sendTelegram(req);
    QCOMPARE(service.lastError(), QString());
    QCOMPARE(ret.telegramHex().value(), QByteArray("Received ") + hugePayload);
    QCOMPARE(ret.telegramBase64().constData(), QByteArray("Received " + hugePayload).constData());
}

void WsdlDocumentTest::testServerDelayedCall()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();
    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());

    const QByteArray ret = service.delayedAddEmployee(addEmployeeParameters());
    QCOMPARE(service.lastError(), QString());
    QCOMPARE(QString::fromLatin1(ret.constData()), QString::fromLatin1("delayed reply works"));
}

void WsdlDocumentTest::testSyncCallAfterServerDelayedCall()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();
    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());

    const QByteArray ret = service.delayedAddEmployee(addEmployeeParameters());
    QCOMPARE(service.lastError(), QString());
    QCOMPARE(QString::fromLatin1(ret.constData()), QString::fromLatin1("delayed reply works"));

    const QByteArray ret2 = service.addEmployee(addEmployeeParameters());
    QCOMPARE(service.lastError(), QString());
    QCOMPARE(QString::fromLatin1(ret2.constData()), QString::fromLatin1("added David Faure"));
}

void WsdlDocumentTest::testServerTwoDelayedCalls()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();
    MyWsdlDocument service;
    service.setEndPoint(server->endPoint());

    connect(&service, SIGNAL(delayedAddEmployeeDone(QByteArray)),
            this, SLOT(slotDelayedAddEmployeeDone(QByteArray)));
    m_expectedDelayedCalls = 2;

    // Interestingly, this doesn't test what I thought it would test.
    // Making two async calls means QNAM will connect two different client sockets to the server,
    // which means we have two different KDSoapServerSockets, not one.
    // So basically we can't test "disable socket while waiting for delayed response"
    // in KDSoapServerSocket, because QNAM already protects us from the old mistake of
    // "sending two requests without waiting for the response of the first request".
    service.asyncDelayedAddEmployee(addEmployeeParameters());
    service.asyncDelayedAddEmployee(addEmployeeParameters());

    m_eventLoop.exec();

    QCOMPARE(service.lastError(), QString());
    const QString expected = QString::fromLatin1("delayed reply works");
    QCOMPARE(QString::fromLatin1(m_delayedData.at(0).constData()), expected);
    QCOMPARE(QString::fromLatin1(m_delayedData.at(1).constData()), expected);
}

// Same as testSequenceInResponse (thomas-bayer.wsdl), but as a server test, by calling DocServer on a different path
void WsdlDocumentTest::testServerDifferentPath()
{
    TestServerThread<DocServer> serverThread;
    DocServer* server = serverThread.startThread();

    NamesServiceService serv;
    QString endPoint = server->endPoint(); // ends with "/xml"
    endPoint += QLatin1String("/../xml/./NamesService?foo");
    serv.setEndPoint(endPoint);
    const QStringList countries = serv.getCountries().country(); // the wsdl should have named it "countries"...
    QCOMPARE(countries.count(), 2);
    QCOMPARE(countries[0], QString::fromLatin1("Great Britain"));
    QCOMPARE(countries[1], QString::fromLatin1("Ireland"));

}

QTEST_MAIN(WsdlDocumentTest)

#include "test_wsdl_document.moc"
