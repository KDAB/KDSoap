/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "test_optionaltype_regular.h"
#include <QTest>
#include <QDebug>

TestRegularApi::TestRegularApi()
{
}

void TestRegularApi::test()
{
    KDAB::TNS__TestOperationResponse1 resp;
    QCOMPARE(resp.out(), QString());
    QString newVal("newval");
    resp.setOut(newVal);
    QCOMPARE(resp.out(), newVal);
}

void TestRegularApi::testPolymorphic()
{
    KDAB::TNS__TestOperationResponse1 resp;
    QCOMPARE(resp.out2(), ( KDAB::TNS__PolymorphicClass * )0);
    QCOMPARE(resp.hasValueForOut2(), false);
    KDAB::TNS__PolymorphicClass value;
    value.setValue(QString("newvalue"));
    resp.setOut2(value);
    QVERIFY(resp.out2());
    QCOMPARE(resp.out2()->value(), QString("newvalue"));
    QCOMPARE(resp.hasValueForOut2(), true);

    KDAB::TNS__DerivedClass derivedValue;
    derivedValue.setValue("derived");
    derivedValue.setValue2("derived");
    resp.setOut2(derivedValue);
    QVERIFY(resp.out2());
    QCOMPARE(resp.out2()->value(), QString("derived"));
    QCOMPARE(dynamic_cast<const KDAB::TNS__DerivedClass *>(resp.out2())->value2(), QString("derived"));
}

void TestRegularApi::testSerialize()
{
    // TNS__TestOperationResponse1 has "out3" which is polymorphic AND non-optional
    //    If "out3" is not initialized, then trying to serialize causes a crash.
    //    Verify it doesn't crash.
    KDAB::TNS__TestOperationResponse1 resp;
    resp.serialize("Test");
    QCOMPARE(resp.out3().value(), QString());

    KDAB::TNS__DerivedClass derivedValue;
    derivedValue.setValue("derived");
    derivedValue.setValue2("derived");
    resp.setOut3(derivedValue);
    QCOMPARE(resp.out3().value(), QString("derived"));
}

void TestRegularApi::testRecursiveType() // https://github.com/KDAB/KDSoap/issues/83
{
    KDAB::TNS__TestOperation op;
    KDAB::TNS__RecursiveType in, child;
    child.setName("child");
    in.setChild(child);
    op.setIn(in);
    QCOMPARE(op.in().child()->name(), QString("child"));
}

QTEST_MAIN(TestRegularApi)
