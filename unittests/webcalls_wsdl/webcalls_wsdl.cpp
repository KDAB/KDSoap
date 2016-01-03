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
//#include "KDSoapMessage.h"
//#include "KDSoapValue.h"
#include "wsdl_soapresponder.h"
#include "wsdl_holidays.h"
#include "wsdl_BFGlobalService.h"
#include "wsdl_OrteLookup.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

class WebCallsWSDL : public QObject
{
    Q_OBJECT
public:

private slots:

#if 0 // www.soapclient.com is down...
    // Soap in RPC mode; using WSDL-generated class
    // http://www.soapclient.com/soapclient?fn=soapform&template=/clientform.html&soaptemplate=/soapresult.html&soapwsdl=http://soapclient.com/xml/soapresponder.wsdl
    void testSoapResponder_sync()
    {
        SoapResponder responder;
        responder.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        QString ret = responder.method1(QLatin1String("abc"), QLatin1String("def"));
        QCOMPARE(ret, QString::fromLatin1("Your input parameters are abc and def"));
    }

    void testSoapResponder_async()
    {
        SoapResponder responder;
        QSignalSpy spyDone(&responder, SIGNAL(method1Done(QString)));
        QEventLoop eventLoop;
        connect(&responder, SIGNAL(method1Done(QString)), &eventLoop, SLOT(quit()));
        responder.asyncMethod1(QLatin1String("abc"), QLatin1String("def"));
        eventLoop.exec();
        QCOMPARE(spyDone.count(), 1);
        QCOMPARE(spyDone[0][0].toString(), QString::fromLatin1("Your input parameters are abc and def"));
    }
#endif

    // Soap in Document mode.

    void testHolidays_wsdl_soap()
    {
        const int year = 2009;
        USHolidayDates holidays;
        TNS__GetValentinesDay parameters;
        parameters.setYear(year);
        TNS__GetValentinesDayResponse response = holidays.getValentinesDay(parameters);
        qDebug() << response.getValentinesDayResult();
        const QString dateString = response.getValentinesDayResult().date().toString(Qt::ISODate);
        QCOMPARE(dateString, QString::fromLatin1("2009-02-14"));
    }

    void testParallelAsyncRequests()
    {
        USHolidayDates holidays;
        QStringList expectedResults;
        for (int year = 2007; year < 2010; ++year) {
            TNS__GetValentinesDay parameters;
            parameters.setYear(year);
            holidays.asyncGetValentinesDay(parameters);
            expectedResults += QString::fromLatin1("%1-02-14T00:00:00").arg(year);
        }
        connect(&holidays, SIGNAL(getValentinesDayDone(TNS__GetValentinesDayResponse)),
                this,  SLOT(slotGetValentinesDayDone(TNS__GetValentinesDayResponse)));
        m_eventLoop.exec();

        //qDebug() << m_resultsReceived;

        // Order of the replies is undefined.
        m_resultsReceived.sort();
        QCOMPARE(m_resultsReceived, expectedResults);
    }

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
        //lookup.setSoapVersion(KDSoapClientInterface::SOAP1_2);
        TNS__OrteStartWith args;
        args.setPrefix(QLatin1String("Berl"));
        TNS__OrteStartWithResponse resp = lookup.orteStartWith(args);
        if (!lookup.lastError().isEmpty())
            qWarning("%s", qPrintable(lookup.lastError()));
        QCOMPARE(resp.orteStartWithResult(), QString::fromLatin1("Berlin;Berlstedt"));
    }

    // TODO: a great example for complex returned structures:
    // http://www.holidaywebservice.com/Holidays/HolidayService.asmx?op=GetHolidaysForYear

protected slots:
    void slotGetValentinesDayDone(const TNS__GetValentinesDayResponse& response)
    {
        m_resultsReceived << response.getValentinesDayResult().toString(Qt::ISODate);
        if (m_resultsReceived.count() == 3)
            m_eventLoop.quit();
    }

private:
    QEventLoop m_eventLoop;
    QStringList m_resultsReceived;
};

QTEST_MAIN(WebCallsWSDL)

#include "webcalls_wsdl.moc"

