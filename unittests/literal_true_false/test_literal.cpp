/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "wsdl_literal.h"
#include "httpserver_p.h"
#include <QTest>
#include <QEventLoop>
#include <QDebug>

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
        // server down, just a compile check
        __BooleanResponseType resp;
    }
};

QTEST_MAIN(LiteralTest)

#include "test_literal.moc"
