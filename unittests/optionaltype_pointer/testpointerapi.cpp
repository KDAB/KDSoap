/****************************************************************************
** Copyright (C) 2014-2019 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** Author: Ville Voutilainen <ville.voutilainen gmail.com>
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
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

#include "testpointerapi.h"
#include <QTest>
#include <QDebug>
#include <QList>

TestPointerApi::TestPointerApi()
{
}

void TestPointerApi::test()
{
    TNS__TestOperationResponse1 resp;
    QCOMPARE(resp.out(), (QString *)0);
    QString newVal("newval");
    resp.setOut(newVal);
    QCOMPARE(*resp.out(), newVal);
}

void TestPointerApi::testOptionalArray()
{
    TNS__TestOperationResponse1 resp;
    QVERIFY(!resp.outOptionalArray());
    const QList<qint64> valIn(QList<qint64>() << 1 << 2 << 3);
    resp.setOutOptionalArray(valIn);

    TNS__TestOperationResponse1 resp2;
    resp2.deserialize(resp.serialize("array"));

    QVERIFY(resp2.outOptionalArray());
    QCOMPARE(valIn, *resp2.outOptionalArray());
}

void TestPointerApi::testPolymorphic()
{
    TNS__TestOperationResponse1 resp;
    QCOMPARE(resp.out2(), (TNS__PolymorphicClass*)0);
    QCOMPARE(resp.hasValueForOut2(), false);
    TNS__PolymorphicClass value;
    value.setValue(QString("newvalue"));
    resp.setOut2(value);
    QVERIFY(resp.out2());
    QCOMPARE(resp.out2()->value(), QString("newvalue"));
    QCOMPARE(resp.hasValueForOut2(), true);


    TNS__DerivedClass derivedValue;
    derivedValue.setValue("derived");
    derivedValue.setValue2("derived");
    resp.setOut2(derivedValue);
    QVERIFY(resp.out2());
    QCOMPARE(resp.out2()->value(), QString("derived"));
    QCOMPARE(dynamic_cast<const TNS__DerivedClass*>(resp.out2())->value2(), QString("derived"));
}

void TestPointerApi::testSerialize()
{
    // TNS__TestOperationResponse1 has "out3" which is polymorphic AND non-optional
    //    If "out3" is not initialized, then trying to serialize causes a crash.
    //    Verify it doesn't crash.
    TNS__TestOperationResponse1 resp;
    resp.serialize("Test");
    QCOMPARE(resp.out3().value(), QString());

    TNS__DerivedClass derivedValue;
    derivedValue.setValue("derived");
    derivedValue.setValue2("derived");
    resp.setOut3(derivedValue);
    QCOMPARE(resp.out3().value(), QString("derived"));
}

QTEST_MAIN(TestPointerApi)
