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

      const KDSoapValue& value = state.serialize("State");
      const QByteArray& actualXml = value.toXml();

      // Serialized XML must contain "idleState"
      QVERIFY(actualXml.contains("idleState"));
    }

    void testMustContainSeanceRemovingState()
    {
      TNS__State state;
      TNS__SeanceRemovingState seanceRemovingState;
      state.setSeanceRemovingState(seanceRemovingState);

      const KDSoapValue& value = state.serialize("State");
      const QByteArray& actualXml = value.toXml();

      // Serialized XML must contain "seanceRemovingState"
      QVERIFY(actualXml.contains("seanceRemovingState"));
    }
};

QTEST_MAIN(EmptyElementTest)

#include "empty_element_wsdl.moc"
