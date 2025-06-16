/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapValue.h"
#include "httpserver_p.h"
#include "wsdl_jobs.h"
#include <QDebug>
#include <QEventLoop>
#include <QTest>

using namespace KDSoapUnitTestHelpers;

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
