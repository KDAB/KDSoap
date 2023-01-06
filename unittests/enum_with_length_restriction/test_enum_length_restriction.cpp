/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2014-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "httpserver_p.h"
#include "wsdl_test_enum.h"
#include <QTest>

class TestEnum : public QObject
{
    Q_OBJECT
public:
    explicit TestEnum();

private slots:
    void test();
};

using namespace KDSoapUnitTestHelpers;

TestEnum::TestEnum()
{
}

void TestEnum::test()
{
}

QTEST_MAIN(TestEnum)

#include "test_enum_length_restriction.moc"
