/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
