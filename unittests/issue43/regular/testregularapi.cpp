#include "testregularapi.h"
#include <QtTest/QtTest>
#include <QDebug>

TestRegularApi::TestRegularApi()
{
}

void TestRegularApi::test()
{
    TNS__TestOperationResponse1 resp;
    QCOMPARE(resp.out(), QString());
    QString newVal("newval");
    resp.setOut(newVal);
    QCOMPARE(resp.out(), newVal);
}

QTEST_MAIN(TestRegularApi)
