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
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

class WebCalls : public QObject
{
    Q_OBJECT
public:

public slots:
    void slotFinished(KDSoapPendingCallWatcher *watcher)
    {
        qDebug() << Q_FUNC_INFO;
        m_returnMessage = watcher->returnMessage();
        m_eventLoop.quit();
    }

private slots:

    // Soap in Document mode.

    void testAddIntegers_async()
    {
        qDebug() << Q_FUNC_INFO << "this=" << this << "thread()=" << thread() << "currentThread=" << QThread::currentThread() << "main thread=" << qApp->thread();
        const QString endPoint = QString::fromLatin1("http://www.mathertel.de/AJAXEngine/S02_AJAXCoreSamples/CalcService.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/CalcFactors/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.setQualified(true);
        message.addArgument(QLatin1String("number1"), 42);
        message.addArgument(QLatin1String("number2"), 43);
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("AddInteger"), message/*, action*/);
        qDebug() << "pendingCall created";
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        qDebug() << "Created watcher" << watcher;
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        QVERIFY2(!m_returnMessage.isFault(), qPrintable(m_returnMessage.faultAsString()));
        QCOMPARE(m_returnMessage.arguments().first().value().toInt(), 85);
        QCOMPARE(watcher->returnValue().toInt(), 85);
    }

    void testAddIntegers_sync()
    {
        const QString endPoint = QString::fromLatin1("http://www.mathertel.de/AJAXEngine/S02_AJAXCoreSamples/CalcService.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/CalcFactors/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.setQualified(true);
        message.addArgument(QLatin1String("number1"), 42);
        message.addArgument(QLatin1String("number2"), 43);
        KDSoapMessage ret = client.call(QLatin1String("AddInteger"), message);
        QVERIFY2(!ret.isFault(), qPrintable(ret.faultAsString()));
        QCOMPARE(ret.arguments().first().value().toInt(), 85);
    }

    void testHolidays()
    {
        const int year = 2009;
        const QString endPoint = QString::fromLatin1("http://www.holidaywebservice.com/Holidays/US/Dates/USHolidayDates.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.27seconds.com/Holidays/US/Dates/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.setQualified(true);
        message.addArgument(QLatin1String("year"), year);
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("GetValentinesDay"), message/*, action*/);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        if (m_returnMessage.isFault()) {
            qDebug() << m_returnMessage.faultAsString();
            QVERIFY(!m_returnMessage.isFault());
        }
        QCOMPARE(m_returnMessage.arguments().first().value(), QVariant(QString::fromLatin1("2009-02-14T00:00:00")));
    }

    void testHolidaysv2()
    {
        const int year = 2009;
        const QString endPoint = QString::fromLatin1("http://www.holidaywebservice.com/HolidayService_v2/HolidayService2.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.holidaywebservice.com/HolidayService_v2/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.setQualified(true);
        message.addArgument(QLatin1String("countryCode"), QLatin1String("UnitedStates")); // http://www.holidaywebservice.com/ServicesAvailable_HolidayService2_Country-Enum.aspx
        message.addArgument(QLatin1String("holidayCode"), QLatin1String("VALENTINES-DAY"));          // http://www.holidaywebservice.com/ServicesAvailable_HolidayService2_HolidayCode-Object.aspx
        message.addArgument(QLatin1String("year"), year);
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("GetHolidayDate"), message/*, action*/);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        if (m_returnMessage.isFault()) {
            qDebug() << m_returnMessage.faultAsString();
            QVERIFY(!m_returnMessage.isFault());
        }
        QCOMPARE(m_returnMessage.arguments().first().value(), QVariant(QString::fromLatin1("2009-02-14T00:00:00")));
    }

    void testConnectionError()
    {
        const QString endPoint = QString::fromLatin1("http://127.0.0.1:19582");
        const QString messageNamespace = QString::fromLatin1("incorrect, just for testing");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("bstrParam1"), QLatin1String("abc"));
        message.addArgument(QLatin1String("bstrParam2"), QLatin1String("def"));
        KDSoapMessage ret = client.call(QLatin1String("Method1"), message);
        QVERIFY(ret.isFault());
        QCOMPARE(ret.faultAsString(), QString::fromLatin1("Fault code 1: Connection refused"));
    }

    void testOrteLookup()
    {
        const QString endPoint = QString::fromLatin1("http://mathertel.de/AJAXEngine/S02_AJAXCoreSamples/OrteLookup.asmx?WSDL");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/OrteLookup/");
        const QString action = QString::fromLatin1("");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.setQualified(true); // so that the prefix element is qualified
        message.addArgument(QLatin1String("prefix"), QLatin1String("Berl"));
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("OrteStartWith"), message, action);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        //qDebug() << m_returnMessage;

        QVERIFY2(!m_returnMessage.isFault(), qPrintable(m_returnMessage.faultAsString()));
        const QString retVal = m_returnMessage.arguments()[0].value().toString();
        QCOMPARE(retVal, QString::fromLatin1("Berlin;Berlstedt"));
    }

    // TODO: a great example for complex returned structures:
    // http://www.holidaywebservice.com/Holidays/HolidayService.asmx?op=GetHolidaysForYear

private:
    QEventLoop m_eventLoop;
    KDSoapMessage m_returnMessage;
};

QTEST_MAIN(WebCalls)

#include "webcalls.moc"

