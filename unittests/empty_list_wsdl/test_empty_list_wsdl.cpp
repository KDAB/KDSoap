/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "wsdl_empty_list.h"

#include <QTest>

class EmptyListTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testMustReturnEmptyListWhenXmlValueIsEmpty()
    {
        NS2__Orientation list;
        KDSoapValue soapValue;
        soapValue.setValue(QVariant::fromValue(QString("")));
        list.deserialize(soapValue);
        QVERIFY(list.entries().empty());
    }

    void testMustReturnEmptyListWhenQVariantIsNull()
    {
        NS2__Orientation list;
        QVariant null;
        KDSoapValue soapValue;
        soapValue.setValue(null);
        list.deserialize(soapValue);
        QVERIFY(list.entries().empty());
    }

    void testMustReturnEmptyListWhenXmlContainsOnlySpaces()
    {
        NS2__Orientation list;
        KDSoapValue soapValue;
        soapValue.setValue(QVariant::fromValue(QString("    \t   \t")));
        list.deserialize(soapValue);
        QVERIFY(list.entries().empty());
    }
};

QTEST_MAIN(EmptyListTest)

#include "test_empty_list_wsdl.moc"
