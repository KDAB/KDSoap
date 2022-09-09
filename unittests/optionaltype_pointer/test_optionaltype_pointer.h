/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2014-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef TEST_OPTIONALTYPE_POINTER_H
#define TEST_OPTIONALTYPE_POINTER_H

#include "wsdl_test.h"
#include <QObject>

class TestPointerApi : public QObject
{
    Q_OBJECT
public:
    explicit TestPointerApi();

private slots:
    void test();
    void testOptionalArray();
    void testPolymorphic();
    void testSerialize();

private:
};

#endif // TEST_OPTIONALTYPE_POINTER_H
