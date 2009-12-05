#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapPendingCallWatcher.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

class TestObject : public QObject
{
    Q_OBJECT
public:

public slots:
    void slotFinished(KDSoapPendingCallWatcher* watcher)
    {
        m_returnValue = watcher->returnValue();
        m_eventLoop.quit();
    }

private slots:
    void testHolidays()
    {
        const int year = 2009;
        const QString hostname = QString::fromLatin1("www.27seconds.com");
        const QString path = QString::fromLatin1("/Holidays/US/Dates/USHolidayDates.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.27seconds.com/Holidays/US/Dates/");
        KDSoapClientInterface client(hostname, path, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("year"), year);
        KDSoapPendingCall pendingCall = client.asyncCall("GetValentinesDay", message/*, action*/);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        // TODO how are we supposed to know / tell that it's a datetime?
        QCOMPARE(m_returnValue, QVariant("2009-02-14T00:00:00.0000000-05:00"));
    }
private:
    QEventLoop m_eventLoop;
    QVariant m_returnValue;
};

QTEST_MAIN(TestObject)

#include "webcalls.moc"
