#include <QCoreApplication>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"

#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const int year = 2009;

    const QString endPoint = QLatin1String("http://www.holidaywebservice.com/Holidays/US/Dates/USHolidayDates.asmx");
    const QString messageNamespace = QLatin1String("http://www.27seconds.com/Holidays/US/Dates/");
    KDSoapClientInterface client(endPoint, messageNamespace);

    KDSoapMessage message;
    message.setQualified(true);
    message.addArgument(QLatin1String("year"), year);

    qDebug("Looking up the date of Valentine's Day in %i...", year);

    KDSoapMessage response = client.call(QLatin1String("GetValentinesDay"), message);
    if (response.isFault())
        printf("%s\n", qPrintable(response.faultAsString()));
    else
        printf("%s\n", qPrintable(response.arguments()[0].value().toString()));

    return 0;
}
