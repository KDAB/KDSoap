/****************************************************************************
** Copyright (C) 2014-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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
