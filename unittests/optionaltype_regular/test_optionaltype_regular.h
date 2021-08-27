/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2014-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#ifndef TESTREGULARAPI_H
#define TESTREGULARAPI_H

#include <QObject>
#include "wsdl_test.h"

class TestRegularApi : public QObject
{
    Q_OBJECT
public:
    explicit TestRegularApi();

private slots:
    void test();
    void testPolymorphic();
    void testSerialize();
    void testRecursiveType();

private:
};

#endif // TESTREGULARAPI_H
