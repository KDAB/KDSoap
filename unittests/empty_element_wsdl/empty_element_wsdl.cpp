/****************************************************************************
** Copyright (C) 2010-2018 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "KDSoapValue.h"
#include "wsdl_empty_element.h"

#include <QtTest>

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
