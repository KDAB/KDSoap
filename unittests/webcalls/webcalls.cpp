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
    void slotFinished(KDSoapPendingCallWatcher* watcher)
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
        message.addArgument(QLatin1String("number1"), 42);
        message.addArgument(QLatin1String("number2"), 43);
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("AddInteger"), message/*, action*/);
        qDebug() << "pendingCall created";
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        qDebug() << "Created watcher" << watcher;
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        QVERIFY(!m_returnMessage.isFault());
        QCOMPARE(m_returnMessage.arguments().first().value().toInt(), 85);
        QCOMPARE(watcher->returnValue().toInt(), 85);
    }

    void testAddIntegers_sync()
    {
        const QString endPoint = QString::fromLatin1("http://www.mathertel.de/AJAXEngine/S02_AJAXCoreSamples/CalcService.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/CalcFactors/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("number1"), 42);
        message.addArgument(QLatin1String("number2"), 43);
        KDSoapMessage ret = client.call(QLatin1String("AddInteger"), message);
        QCOMPARE(ret.arguments().first().value().toInt(), 85);
    }

    void testHolidays()
    {
        const int year = 2009;
        const QString endPoint = QString::fromLatin1("http://www.27seconds.com/Holidays/US/Dates/USHolidayDates.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.27seconds.com/Holidays/US/Dates/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("year"), year);
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("GetValentinesDay"), message/*, action*/);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        QVERIFY(!m_returnMessage.isFault());
        QCOMPARE(m_returnMessage.arguments().first().value(), QVariant(QString::fromLatin1("2009-02-14T00:00:00.0000000-05:00")));
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
        QCOMPARE(ret.faultAsString(), QString::fromLatin1("Fault code: 1\nFault description: Connection refused ()"));
    }

    void testOrteLookup()
    {
        const QString endPoint = QString::fromLatin1("http://mathertel.de/AJAXEngine/S02_AJAXCoreSamples/OrteLookup.asmx?WSDL");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/OrteLookup/");
        const QString action = QString::fromLatin1("");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("prefix"), QLatin1String("Berl"));
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("OrteStartWith"), message, action);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        //qDebug() << m_returnMessage;

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

