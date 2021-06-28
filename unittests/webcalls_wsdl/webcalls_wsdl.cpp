/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "wsdl_BLZService.h"
#include "wsdl_BFGlobalService.h"
#include "wsdl_OrteLookup.h"
#include <QTest>
#include <QSignalSpy>
#include <QEventLoop>
#include <QDebug>

class WebCallsWSDL : public QObject
{
    Q_OBJECT
public:
private slots:

#if 0 // 2020-12-02 bad example, it's returning malformed XML!
    // Soap in RPC mode; using WSDL-generated class
    // http://www.soapclient.com/soapclient?fn=soapform&template=/clientform.html&soaptemplate=/soapresult.html&soapwsdl=http://soapclient.com/xml/soapresponder.wsdl
    void testSoapResponder_sync()
    {
        EmptySA responder;
        responder.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        QString ret = responder.echoString(QLatin1String("abc"));
        QCOMPARE(ret, QString::fromLatin1("Your input parameters are abc and def"));
    }

    void testSoapResponder_async()
    {
        EmptySA responder;
        QSignalSpy spyDone(&responder, &EmptySA::echoStringDone);
        QEventLoop eventLoop;
        connect(&responder, &EmptySA::echoStringDone, &eventLoop, &QEventLoop::quit);
        responder.asyncEchoString(QLatin1String("abc"));
        eventLoop.exec();
        QCOMPARE(spyDone.count(), 1);
        QCOMPARE(spyDone[0][0].toString(), QString::fromLatin1("Your input parameters are abc and def"));
    }
#endif

    // Soap in Document mode.

#if 0 // 2020-02-12: http://www.thomas-bayer.com/axis2/services/BLZService?wsdl is down
    void testBLZService_wsdl_soap()
    {
        BLZService::BLZServiceSOAP11Binding service;
        TNS__GetBankType getBankType;
        getBankType.setBlz("20130600"); // found on http://www.thebankcodes.com/blz/bybankname.php
        TNS__GetBankResponseType response = service.getBank(getBankType);
        QCOMPARE(response.details().bic(), QString::fromLatin1("BARCDEH1XXX"));
        QCOMPARE(response.details().bezeichnung(), QString::fromLatin1("Barclaycard Barclays Bank"));
        QCOMPARE(response.details().ort(), QString::fromLatin1("Hamburg"));
        QCOMPARE(response.details().plz(), QString::fromLatin1("22702"));
    }

    void testParallelAsyncRequests()
    {
        BLZService::BLZServiceSOAP11Binding service;
        const QStringList expectedResults = {"BARCDEH1XXX", "BEBEDEBBXXX", "BEVODEBBXXX"};

        for (const char* blz : { "10020000", "20130600", "10090000" }) {
            TNS__GetBankType getBankType;
            getBankType.setBlz(QString::fromLatin1(blz));
            service.asyncGetBank(getBankType);
        }
        connect(&service, &BLZService::BLZServiceSOAP11Binding::getBankDone, this, &WebCallsWSDL::slotGetBankDone);
        connect(&service, &BLZService::BLZServiceSOAP11Binding::getBankError, this, &WebCallsWSDL::slotGetBankError);
        m_eventLoop.exec();

        //qDebug() << m_resultsReceived;

        // Order of the replies is undefined.
        m_resultsReceived.sort();
        QCOMPARE(m_resultsReceived, expectedResults);
    }
#endif

    void testBetFair() // SOAP-14
    {
        TYPES__LoginReq loginReq;
        TYPES__LoginResp loginResp;
        BFGlobalService globalService;
        loginReq.setUsername(QLatin1String("user"));
        loginReq.setPassword(QLatin1String("pass"));
        loginReq.setProductId(82);
        loginReq.setIpAddress(QLatin1String("0"));
        loginReq.setVendorSoftwareId(0);

        TNS__Login loginParam;
        loginParam.setRequest(loginReq);
        TNS__LoginResponse lResp;
        lResp.setResult(loginResp);

        // Don't make the call, it errors out (invalid login/pass, but also restricted country)
        if (false) {
            lResp = globalService.login(loginParam);
            qDebug() << globalService.lastError();
        }
    }

    void testOrteLookup()
    {
        // TODO OrteLookup::OrteLookupSoap12 lookup;
        OrteLookup::OrteLookupSoap lookup;
        // lookup.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        TNS__OrteStartWith args;
        args.setPrefix(QLatin1String("Berl"));
        TNS__OrteStartWithResponse resp = lookup.orteStartWith(args);
        if (!lookup.lastError().isEmpty()) {
            qWarning("%s", qPrintable(lookup.lastError()));
        }
        QCOMPARE(resp.orteStartWithResult(), QString::fromLatin1("Berlin;Berlstedt"));
    }

protected slots:
    void slotGetBankDone(const TNS__GetBankResponseType &response)
    {
        m_resultsReceived << response.details().bic();
        if (m_resultsReceived.count() == 3) {
            m_eventLoop.quit();
        }
    }

    void slotGetBankError(const KDSoapMessage &msg)
    {
        m_resultsReceived << msg.faultAsString();
        if (m_resultsReceived.count() == 3) {
            m_eventLoop.quit();
        }
    }

private:
    QEventLoop m_eventLoop;
    QStringList m_resultsReceived;
};

QTEST_MAIN(WebCallsWSDL)

#include "webcalls_wsdl.moc"
