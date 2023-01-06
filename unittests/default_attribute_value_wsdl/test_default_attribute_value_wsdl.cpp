/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapValue.h"
#include "wsdl_default_attribute_value.h"

#include <QTest>

class DefaultAttributeValueTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testMustReturnDefaultValueWhenValueNotSet()
    {
        TNS__State state;
        QCOMPARE(state.errorDescription(), QString("defaultError"));
    }

    void testMustReturnActualValueWhenValueIsSet()
    {
        TNS__State state;
        const QString description("notDefault");
        state.setErrorDescription(description);
        QCOMPARE(state.errorDescription(), description);
    }

    void testDefaultBoolValue()
    {
        TNS__State state;
        QCOMPARE(state.isCancelled(), true);
    }

    void testDefaultIntValue()
    {
        TNS__State state;
        QCOMPARE(state.intValue(), 1985);
    }

    void testDefaultDoubleValue()
    {
        TNS__State state;
        QCOMPARE(state.doubleValue(), 1985.145);
    }

    void testDefaultByteArrayValue()
    {
        TNS__State state;
        QCOMPARE(state.byteArrayValue(), QByteArray("0xFFAA"));
    }

    void testDefaultEnumerationValue()
    {
        TNS__State state;
        QCOMPARE(state.enumerationValue(), TNS__SynchronizationType(TNS__SynchronizationType::MIDDLE));
    }

    void testDefaultTimeValue()
    {
        TNS__State state;
        QCOMPARE(state.timeValue(), QTime(9, 30, 10));
    }
    void testDefaultDateValue()
    {
        TNS__State state;
        QCOMPARE(state.dateValue(), QDate(2002, 1, 24));
    }

    void testDefaultDateTimeValue()
    {
        TNS__State state;
        QCOMPARE(state.dateTimeValue(), KDDateTime(QDateTime(QDate(2002, 5, 30), QTime(9, 30, 10))));
    }

    void testMustNotSerializeWhenValueNotSet()
    {
        TNS__State state;
        const QString description("notDefault");
        state.setErrorDescription(description);

        const KDSoapValue &value = state.serialize("State");
        const QByteArray &actualXml = value.toXml();

        QVERIFY(actualXml.contains("errorDescription"));
        QVERIFY(!actualXml.contains("isCancelled"));
        QVERIFY(!actualXml.contains("intValue"));
        QVERIFY(!actualXml.contains("doubleValue"));
        QVERIFY(!actualXml.contains("byteArrayValue"));
        QVERIFY(!actualXml.contains("enumerationValue"));
    }
};

QTEST_MAIN(DefaultAttributeValueTest)

#include "test_default_attribute_value_wsdl.moc"
