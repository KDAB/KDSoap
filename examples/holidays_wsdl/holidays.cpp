#include <QCoreApplication>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapPendingCallWatcher.h"

#include <QDebug>

#include "wsdl_holidays.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const int year = 2009;
    qDebug("Looking up the date of easter in %i...", year);

    USHolidayDates holidays;
    TNS__GetValentinesDay parameters;
    parameters.setYear(year);
    TNS__GetValentinesDayResponse response = holidays.getValentinesDay(parameters);

    qDebug("%s", qPrintable(response.getValentinesDayResult().toString()));

    return 0;
}
