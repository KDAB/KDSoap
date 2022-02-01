/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include <QTest>
#include <QDebug>
#include "wsdl_test.h"

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
