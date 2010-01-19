#include <QCoreApplication>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapPendingCallWatcher.h"

#include <QDebug>

// TODO a button for an async call, a button for a sync call,
// and some background operation that will stop working during the sync call
// (or just a mouseover effect?)

class TestObject : public QObject
{
    Q_OBJECT
public:
    TestObject() : QObject(0) {}

public slots:
    void slotFinished(KDSoapPendingCallWatcher* watcher)
    {
        qDebug() << watcher->returnValue().toString();
        QCoreApplication::instance()->quit();
    }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const int year = 2009;

    const QString endPoint = QString::fromLatin1("http://www.27seconds.com/Holidays/US/Dates/USHolidayDates.asmx");
    const QString messageNamespace = QString::fromLatin1("http://www.27seconds.com/Holidays/US/Dates/");
    KDSoapClientInterface client(endPoint, messageNamespace);

    KDSoapMessage message;
    message.addArgument(QLatin1String("year"), year);

    qDebug("Looking up the date of easter in %i...", year);

    //KDSoapMessage /*KDSoapResponse?*/ response = client.call("GetValentinesDay", message /*, action*/);

    //qDebug() << response;

    KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("GetValentinesDay"), message);
    TestObject test;
    KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, &test);

    QObject::connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                     &test, SLOT(slotFinished(KDSoapPendingCallWatcher*)));

    return app.exec();
}

#include "holidays.moc"
