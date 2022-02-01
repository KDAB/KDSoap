/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2019-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
