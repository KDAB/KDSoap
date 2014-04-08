#include "testpointerapi.h"
#include <QtTest/QtTest>
#include <QDebug>

TestPointerApi::TestPointerApi()
{
}

void TestPointerApi::test()
{
    TNS__TestOperationResponse1 resp;
    QCOMPARE(resp.out(), (QString*)0);
    QString newVal("newval");
    resp.setOut(newVal);
    QCOMPARE(*resp.out(), newVal);
}

QTEST_MAIN(TestPointerApi)
