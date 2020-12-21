/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "KDSoapMessageWriter_p.h"
#include <QTest>

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

    const QByteArray faultString("Fault");
    const QByteArray startTag("<soap:" + faultString +">");
    const QByteArray endTag("</soap:" + faultString + ">");

    const QByteArray result = writer.messageToXml(faultMessage, faultString, KDSoapHeaders(), QMap<QString, KDSoapMessage>());

    QVERIFY(result.contains(startTag));
    QVERIFY(result.contains(endTag));
  }
};

QTEST_MAIN(FaultNamespace)

#include "fault_namespace.moc"

