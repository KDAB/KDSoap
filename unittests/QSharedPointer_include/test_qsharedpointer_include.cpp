/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2019-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "wsdl_test_qsharedpointer_include_wsdl.h"

#include <QTest>

class RightInclude : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCompiled()
    {
        Hello_Service service;
    }
};

QTEST_MAIN(RightInclude)

#include "test_qsharedpointer_include.moc"
