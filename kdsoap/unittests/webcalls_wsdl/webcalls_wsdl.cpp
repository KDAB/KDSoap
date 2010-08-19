//#include "KDSoapClientInterface.h"
//#include "KDSoapMessage.h"
//#include "KDSoapValue.h"
#include "wsdl_soapresponder.h"
#include "wsdl_holidays.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

class WebCallsWSDL : public QObject
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

    void testHolidays_wsdl()
    {
        const int year = 2009;
        USHolidayDates holidays;
        TNS__GetValentinesDay parameters;
        parameters.setYear(year);
        TNS__GetValentinesDayResponse response = holidays.getValentinesDay(parameters);
        QCOMPARE(response.getValentinesDayResult().date().toString(Qt::ISODate),
                 QString::fromLatin1("2009-02-14"));
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

