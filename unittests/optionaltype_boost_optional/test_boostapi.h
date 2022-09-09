/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef TEST_BOOSTAPI_H
#define TEST_BOOSTAPI_H

#include "wsdl_test.h"
#include <QObject>

class TestBoostApi : public QObject
{
    Q_OBJECT
public:
    explicit TestBoostApi();

private slots:
    void test();
    void testPolymorphic();
    void testSerialize();

private:
};

#endif // TEST_BOOSTAPI_H
