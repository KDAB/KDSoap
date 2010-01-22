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
        m_returnMessage = watcher->returnMessage();
        m_eventLoop.quit();
    }

private slots:

    // Soap in Document mode.

    void testAddIntegers_async()
    {
        const QString endPoint = QString::fromLatin1("http://www.mathertel.de/AJAXEngine/S02_AJAXCoreSamples/CalcService.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/CalcFactors/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("number1"), 42);
        message.addArgument(QLatin1String("number2"), 43);
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("AddInteger"), message/*, action*/);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
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

    void testFault()
    {
        const QString endPoint = QString::fromLatin1("http://soapclient.com/xml/doesnotexist");
        const QString messageNamespace = QString::fromLatin1("incorrect, just for testing");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("bstrParam1"), QLatin1String("abc"));
        message.addArgument(QLatin1String("bstrParam2"), QLatin1String("def"));
        KDSoapMessage ret = client.call(QLatin1String("Method1"), message);
        qDebug() << ret;
        QVERIFY(ret.isFault());
        QCOMPARE(ret.faultAsString(), QString::fromLatin1("Fault code: SOAP-ENV:Server\nFault description: The parameter is incorrect. (/xml/doesnotexist)"));
    }

    // http://www.service-repository.com/service/wsdl?id=163859
    void testServiceRepositoryCom()
    {
        const QString endPoint = QString::fromLatin1("http://www.thomas-bayer.com/names-service/soap");
        const QString messageNamespace = QString::fromLatin1("http://namesservice.thomas_bayer.com/");
        const QString action = QString::fromLatin1("");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("getCountries"), message, action);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        qDebug() << m_returnMessage;

        // TODO QCOMPARE(m_returnArguments[0], QString::fromLatin1("Great Britain"));
    }

    // TODO: a great example for complex returned structures:
    // http://www.holidaywebservice.com/Holidays/HolidayService.asmx?op=GetHolidaysForYear

private:
    QEventLoop m_eventLoop;
    KDSoapMessage m_returnMessage;
};

QTEST_MAIN(WebCalls)

#include "webcalls.moc"
