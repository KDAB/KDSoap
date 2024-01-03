/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef TEST_ELEMENT_NS_WSDL_H
#define TEST_ELEMENT_NS_WSDL_H

#include "wsdl_test.h"
#include <QObject>

class TestCall : public QObject
{
    Q_OBJECT
public:
    explicit TestCall();

private slots:
    void test();

private:
    PREFIX test_client;
};

#endif // TEST_ELEMENT_NS_WSDL_H
