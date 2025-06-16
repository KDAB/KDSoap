/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDDateTime.h"
#include "KDSoapValue.h"
#include <QTest>
#include <QTimeZone>

class KDDateTimeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testQVariantArgConversion()
    {
        KDDateTime inputDateTime(QDateTime::currentDateTimeUtc());
        inputDateTime.setTimeZone("Z");

        // Add to the value list, which implicitly constructs a QVariant
        // from the KDDateTime...
        KDSoapValueList list;
        list.addArgument("Timestamp", inputDateTime);

        /// Retrieve the KDDateTime from QVariant
        KDDateTime outputDateTime = list.child("Timestamp").value().value<KDDateTime>();

        QCOMPARE(inputDateTime, outputDateTime);

        QCOMPARE(inputDateTime.timeZone(), outputDateTime.timeZone());
        QCOMPARE(inputDateTime.toDateString(), outputDateTime.toDateString());
    }

    void testSetTimeZone_data()
    {
        QTest::addColumn<QString>("timeZone");
        QTest::addColumn<QString>("expected");

        // Example countries and their offsets.

        QTest::newRow("empty") << "" << QDateTime({2020, 1, 1}, {0, 0}, QTimeZone::systemTimeZone()).timeZoneAbbreviation();
        QTest::newRow("Z") << "Z" << "UTC";
        QTest::newRow("US Minor outlying islands") << "-12:00" << "UTC-12:00";
        QTest::newRow("New Zealand (Niue)") << "-11:00" << "UTC-11:00";
        QTest::newRow("Hawaii") << "-10:00" << "UTC-10:00";
        QTest::newRow("Marquesas Islands (French Polynesia)") << "-09:30" << "UTC-09:30";
        QTest::newRow("Alaska") << "-09:00" << "UTC-09:00";
        QTest::newRow("US Pacific") << "-08:00" << "UTC-08:00";
        QTest::newRow("US Mountain") << "-07:00" << "UTC-07:00";
        QTest::newRow("US Central") << "-06:00" << "UTC-06:00";
        QTest::newRow("US Eastern") << "-05:00" << "UTC-05:00";
        QTest::newRow("US Atlantic") << "-04:00" << "UTC-04:00";
        QTest::newRow("Canada Newfoundland") << "-03:30" << "UTC-03:30";
        QTest::newRow("Argentina") << "-03:00" << "UTC-03:00";
        QTest::newRow("Greenland") << "-02:00" << "UTC-02:00";
        QTest::newRow("Cape Verde") << "-01:00" << "UTC-01:00";
        QTest::newRow("United Kingdom 1") << "+00:00" << "UTC";
        QTest::newRow("United Kingdom 2") << "-00:00" << "UTC";
        QTest::newRow("France (Metropolitan)") << "+01:00" << "UTC+01:00";
        QTest::newRow("Greece") << "+02:00" << "UTC+02:00";
        QTest::newRow("Turkey") << "+03:00" << "UTC+03:00";
        QTest::newRow("Iran") << "+03:30" << "UTC+03:30";
        QTest::newRow("Seychelles") << "+04:00" << "UTC+04:00";
        QTest::newRow("Afghanistan") << "+04:30" << "UTC+04:30";
        QTest::newRow("Kazakhstan") << "+05:00" << "UTC+05:00";
        QTest::newRow("India") << "+05:30" << "UTC+05:30";
        QTest::newRow("Bhutan") << "+06:00" << "UTC+06:00";
        QTest::newRow("Myanmar") << "+06:30" << "UTC+06:30";
        QTest::newRow("Cambodia") << "+07:00" << "UTC+07:00";
        QTest::newRow("Western Australia") << "+08:00" << "UTC+08:00";
        QTest::newRow("Western Australia (Eucla)") << "+08:45" << "UTC+08:45";
        QTest::newRow("Japan") << "+09:00" << "UTC+09:00";
        QTest::newRow("Australia (Adelaide)") << "+09:30" << "UTC+09:30";
        QTest::newRow("Australia (Sydney)") << "+10:00" << "UTC+10:00";
        QTest::newRow("Australia (Lord Howe Island)") << "+10:30" << "UTC+10:30";
        QTest::newRow("Australia (Norfolk Island)") << "+11:00" << "UTC+11:00";
        QTest::newRow("New Zealand") << "+12:00" << "UTC+12:00";
        QTest::newRow("New Zealand (Chatham Islands)") << "+12:45" << "UTC+12:45";
        QTest::newRow("Samoa") << "+13:00" << "UTC+13:00";
        QTest::newRow("Kiribati (Line Islands)") << "+14:00" << "UTC+14:00";
    }

    void testSetTimeZone()
    {
        QFETCH(QString, timeZone);
        QFETCH(QString, expected);

        QCOMPARE(KDDateTime::fromDateString("2020-01-01T00:00" + timeZone).timeZoneAbbreviation(), expected);
    }
};

QTEST_MAIN(KDDateTimeTest)

#include "test_kddatetime.moc"
