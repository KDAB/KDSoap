/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "KDSoapValue.h"
#include "KDDateTime.h"
#include <QtTest/QtTest>

class Basic : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testValueSwap()
    {
        static const QLatin1String hello("Hello, World!" );
        KDSoapValue v1( QLatin1String( "v1" ), 10 );
        KDSoapValue v2( QLatin1String( "v2" ), hello );
        v1.swap( v2 );
        QCOMPARE( v1.value().toString(), hello );
        QCOMPARE( v1.name(), QLatin1String( "v2" ) );
        QCOMPARE( v2.value().toInt(), 10 );
        QCOMPARE( v2.name(), QLatin1String( "v1" ) );
        qSwap( v1, v2 );
        QCOMPARE( v1.value().toInt(), 10 );
        QCOMPARE( v2.value().toString(), hello );
#ifndef QT_NO_STL
        using std::swap;
        swap( v2, v1 );
        QCOMPARE( v1.value().toString(), hello );
        QCOMPARE( v2.value().toInt(), 10 );
#endif
    }

    void testDateTime()
    {
        QDateTime qdt(QDate(2010, 12, 31));
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

#include "basic.moc"

