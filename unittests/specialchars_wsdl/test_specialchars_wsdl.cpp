/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "wsdl_test.h"
#include <QDebug>
#include <QTest>

class TestConversion : public QObject
{
    Q_OBJECT
public:
    explicit TestConversion();

private slots:
    void test();
};

// using namespace KDSoapUnitTestHelpers;

TestConversion::TestConversion()
{
}

void TestConversion::test()
{
    KDAB__Employee_Name name;
    Q_UNUSED(name);
}

QTEST_MAIN(TestConversion)

#include "test_specialchars_wsdl.moc"
