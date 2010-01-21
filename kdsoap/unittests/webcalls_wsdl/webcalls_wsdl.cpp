//#include "KDSoapClientInterface.h"
//#include "KDSoapMessage.h"
//#include "KDSoapValue.h"
#include "wsdl_soapresponder.h"
#include "wsdl_thomas-bayer.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

class TestObject : public QObject
{
    Q_OBJECT
public:

private slots:

    // Soap in RPC mode; using WSDL-generated class
    // http://www.soapclient.com/soapclient?fn=soapform&template=/clientform.html&soaptemplate=/soapresult.html&soapwsdl=http://soapclient.com/xml/soapresponder.wsdl
    void testSoapResponder_sync()
    {
        SoapResponder responder;
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

    // Soap in Document mode.

    void testAddIntegers_wsdl()
    {
        NamesServiceService serv;
        qDebug() << serv.getCountries().country(); // TODO countryList()?
    }

    // TODO: a great example for complex returned structures:
    // http://www.holidaywebservice.com/Holidays/HolidayService.asmx?op=GetHolidaysForYear
};

QTEST_MAIN(TestObject)

#include "webcalls_wsdl.moc"
