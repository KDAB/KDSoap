/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapValue.h"
#include "wsdl_empty_element.h"

#include <QTest>

class EmptyElementTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testMustContainIdleState()
    {
        TNS__State state;
        TNS__IdleState idleState;
        state.setIdleState(idleState);

        const KDSoapValue &value = state.serialize("State");
        const QByteArray &actualXml = value.toXml();

        // Serialized XML must contain "idleState"
        QVERIFY(actualXml.contains("idleState"));
    }

    void testMustContainSeanceRemovingState()
    {
        TNS__State state;
        TNS__SeanceRemovingState seanceRemovingState;
        state.setSeanceRemovingState(seanceRemovingState);

        const KDSoapValue &value = state.serialize("State");
        const QByteArray &actualXml = value.toXml();

        // Serialized XML must contain "seanceRemovingState"
        QVERIFY(actualXml.contains("seanceRemovingState"));
    }
};

QTEST_MAIN(EmptyElementTest)

#include "test_empty_element_wsdl.moc"
