#include "testboostapi.h"
#include <QtTest/QtTest>
#include <QDebug>

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

QTEST_MAIN(TestBoostApi)
