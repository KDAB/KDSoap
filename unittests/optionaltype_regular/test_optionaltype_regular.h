/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef TEST_OPTIONALTYPE_REGULAR_H
#define TEST_OPTIONALTYPE_REGULAR_H

#include "wsdl_test.h"
#include <QObject>

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

#endif // TEST_OPTIONALTYPE_REGULAR_H
