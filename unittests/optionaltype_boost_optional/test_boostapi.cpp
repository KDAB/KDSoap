/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "test_boostapi.h"
#include <QDebug>
#include <QTest>

TestBoostApi::TestBoostApi()
{
}

void TestBoostApi::test()
{
    TNS__TestOperationResponse1 resp;
    QCOMPARE(resp.out(), boost::optional<QString>());
    QString newVal("newval");
    resp.setOut(newVal);
    QCOMPARE(*resp.out(), newVal);
}

void TestBoostApi::testPolymorphic()
{
    TNS__TestOperationResponse1 resp;
    // QCOMPARE(&resp.out2().get(), (TNS__PolymorphicClass*)0); // Assertion `this->is_initialized()' failed.
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
    // crashes... QCOMPARE(dynamic_cast<const TNS__DerivedClass*>(&resp.out2().get())->value2(), QString("derived"));
}

void TestBoostApi::testSerialize()
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

QTEST_MAIN(TestBoostApi)
