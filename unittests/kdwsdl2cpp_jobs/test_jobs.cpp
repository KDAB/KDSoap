/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2025 Jonathan Brady <jtjbrady@users.noreply.github.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "wsdl_jobs.h"
#include <QTest>

class LiteralTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testRequest()
    {
        // just a compile check
        Example service;
        TestRequestResponseJob requestResponse(&service);
        TestOneWayJob oneWay(&service);
    }
};

QTEST_MAIN(LiteralTest)

#include "test_jobs.moc"
