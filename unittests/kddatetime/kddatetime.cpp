/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
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

#include "kddatetime.moc"
