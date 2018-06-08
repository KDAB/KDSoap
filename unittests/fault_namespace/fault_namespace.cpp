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

#include "KDSoapMessageWriter_p.h"
#include <QtTest>

class FaultNamespace:
  public QObject
{
  Q_OBJECT
private Q_SLOTS:

  void testFaultMessageNamespace()
  {
    KDSoapMessageWriter writer;
    KDSoapMessage faultMessage;
    faultMessage.setFault(true);
    faultMessage.addArgument("faultCode", "fooCode");

    const QString faultString("Fault");
    const QString startTag("<soap:" + faultString +">");
    const QString endTag("</soap:" + faultString + ">");

    const QByteArray& result = writer.messageToXml(faultMessage, faultString, KDSoapHeaders(), QMap<QString, KDSoapMessage>());

    QVERIFY(result.contains(startTag.toLatin1()));
    QVERIFY(result.contains(endTag.toLatin1()));
  }
};

QTEST_MAIN(FaultNamespace)

#include "fault_namespace.moc"

