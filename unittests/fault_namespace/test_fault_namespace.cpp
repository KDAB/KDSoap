/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapMessageWriter_p.h"
#include <QTest>

class FaultNamespace : public QObject
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
        const QByteArray startTag("<soap:" + faultString + ">");
        const QByteArray endTag("</soap:" + faultString + ">");

        const QByteArray result = writer.messageToXml(faultMessage, faultString, KDSoapHeaders(), QMap<QString, KDSoapMessage>());

        QVERIFY(result.contains(startTag));
        QVERIFY(result.contains(endTag));
    }
};

QTEST_MAIN(FaultNamespace)

#include "test_fault_namespace.moc"
