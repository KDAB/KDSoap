#include <QCoreApplication>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapPendingCallWatcher.h"

#include <QDebug>

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
    // TODO see how it works in the .wsdl; maybe model with KDSoapAction with multiple ctors?
    //const QByteArray action = "http://www.27seconds.com/Holidays/US/Dates/GetValentinesDay";

    qDebug("Looking up the date of easter in %i...", year);

    //KDSoapMessage /*KDSoapResponse?*/ response = client.call("GetValentinesDay", message /*, action*/);

    //qDebug() << response;

    KDSoapPendingCall pendingCall = client.asyncCall("GetValentinesDay", message/*, action*/);
    TestObject test;
    KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, &test);

    QObject::connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                     &test, SLOT(slotFinished(KDSoapPendingCallWatcher*)));

    return app.exec();
}

#include "holidays.moc"
