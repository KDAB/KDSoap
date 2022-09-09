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

class Basic : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testValueSwap()
    {
        static const QLatin1String hello("Hello, World!");
        KDSoapValue v1(QLatin1String("v1"), 10);
        KDSoapValue v2(QLatin1String("v2"), hello);
        v1.swap(v2);
        QCOMPARE(v1.value().toString(), hello);
        QCOMPARE(v1.name(), QLatin1String("v2"));
        QCOMPARE(v2.value().toInt(), 10);
        QCOMPARE(v2.name(), QLatin1String("v1"));
        qSwap(v1, v2);
        QCOMPARE(v1.value().toInt(), 10);
        QCOMPARE(v2.value().toString(), hello);
#ifndef QT_NO_STL
        using std::swap;
        swap(v2, v1);
        QCOMPARE(v1.value().toString(), hello);
        QCOMPARE(v2.value().toInt(), 10);
#endif
    }

    void testDateTime()
    {
        QDateTime qdt(QDate(2010, 12, 31), QTime(0, 0, 0));
        QVERIFY(qdt.isValid());
        KDDateTime kdt(qdt);
        QVERIFY(kdt.isValid());
        QCOMPARE(kdt.toDateString(), QString::fromLatin1("2010-12-31T00:00:00"));
        QVERIFY(kdt.timeZone().isEmpty());
        kdt.setTimeZone(QString::fromLatin1("Z"));
        QCOMPARE(kdt.toDateString(), QString::fromLatin1("2010-12-31T00:00:00Z"));
        kdt = QDateTime(QDate(2011, 03, 15), QTime(23, 59, 59, 999));
        kdt.setTimeZone(QString::fromLatin1("+01:00"));
        QCOMPARE(kdt.toDateString(), QString::fromLatin1("2011-03-15T23:59:59.999+01:00"));
    }
};

QTEST_MAIN(Basic)

#include "test_basic.moc"
