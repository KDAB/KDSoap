/****************************************************************************
** Copyright (C) 2010-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include <QCoreApplication>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapPendingCallWatcher.h"

#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const int year = 2009;

    const QString endPoint = QLatin1String("http://www.27seconds.com/Holidays/US/Dates/USHolidayDates.asmx");
    const QString messageNamespace = QLatin1String("http://www.27seconds.com/Holidays/US/Dates/");
    KDSoapClientInterface client(endPoint, messageNamespace);

    KDSoapMessage message;
    message.addArgument(QLatin1String("year"), year);

    qDebug("Looking up the date of Valentine's Day in %i...", year);

    KDSoapMessage response = client.call(QLatin1String("GetValentinesDay"), message);

    qDebug("%s", qPrintable(response.arguments()[0].value().toString()));

    return 0;
}
