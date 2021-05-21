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
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include <QTest>
#include <QEventLoop>
#include <QDebug>
#include <QThread>

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
        qDebug() << Q_FUNC_INFO << "this=" << this << "thread()=" << thread() << "currentThread=" << QThread::currentThread()
                 << "main thread=" << qApp->thread();
        const QString endPoint = QString::fromLatin1("http://www.mathertel.de/AJAXEngine/S02_AJAXCoreSamples/CalcService.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/CalcFactors/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.setQualified(true);
        message.addArgument(QLatin1String("number1"), 42);
        message.addArgument(QLatin1String("number2"), 43);
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("AddInteger"), message /*, action*/);
        qDebug() << "pendingCall created";
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        qDebug() << "Created watcher" << watcher;
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher *)), this, SLOT(slotFinished(KDSoapPendingCallWatcher *)));
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

#if 0 // 2020-02-12: service unavailable
    void testAsyncLiteralUse()
    {
        const QString endPoint = QString::fromLatin1("http://www.thomas-bayer.com/axis2/services/BLZService");
        const QString messageNamespace = QString::fromLatin1("http://thomas-bayer.com/blz/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.setUse(KDSoapMessage::LiteralUse);
        message.setNamespaceUri(messageNamespace);
        message.setQualified(true);
        message.addArgument("blz", "20130600");
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("getBank"), message/*, action*/);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        if (m_returnMessage.isFault()) {
            qDebug() << m_returnMessage.faultAsString();
            QVERIFY(!m_returnMessage.isFault());
        }
        KDSoapValue response = m_returnMessage.arguments().first();
        QCOMPARE(response.childValues().child("ort").value().toString(), QString::fromLatin1("Hamburg"));
    }
#endif

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
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.setQualified(true); // so that the prefix element is qualified
        message.addArgument(QLatin1String("prefix"), QLatin1String("Berl"));
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("OrteStartWith"), message);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher *)), this, SLOT(slotFinished(KDSoapPendingCallWatcher *)));
        m_eventLoop.exec();
        // qDebug() << m_returnMessage;

        QVERIFY2(!m_returnMessage.isFault(), qPrintable(m_returnMessage.faultAsString()));
        const QString retVal = m_returnMessage.arguments()[0].value().toString();
        QCOMPARE(retVal, QString::fromLatin1("Berlin;Berlstedt"));
    }

private:
    QEventLoop m_eventLoop;
    KDSoapMessage m_returnMessage;
};

QTEST_MAIN(WebCalls)

#include "webcalls.moc"
