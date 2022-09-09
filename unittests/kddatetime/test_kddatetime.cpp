/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapValue.h"
#include "KDDateTime.h"
#include <QTest>

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
};

QTEST_MAIN(KDDateTimeTest)

#include "test_kddatetime.moc"
