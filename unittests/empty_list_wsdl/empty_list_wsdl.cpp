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

#include "wsdl_empty_list.h"

#include <QtTest>

class EmptyListTest:
  public QObject
{
Q_OBJECT

private Q_SLOTS:

  void testMustReturnEmptyListWhenXmlValueIsEmpty()
  {
    NS2__Orientation list;
    list.deserialize(QVariant::fromValue(QString("")));
    QVERIFY(list.entries().empty());
  }

  void testMustReturnEmptyListWhenQVariantIsNull()
  {
    NS2__Orientation list;
    QVariant null;
    list.deserialize(null);
    QVERIFY(list.entries().empty());
  }

  void testMustReturnEmptyListWhenXmlContainsOnlySpaces()
  {
    NS2__Orientation list;
    list.deserialize(QVariant::fromValue(QString("    \t   \t")));
    QVERIFY(list.entries().empty());
  }
};

QTEST_MAIN(EmptyListTest)

#include "empty_list_wsdl.moc"
