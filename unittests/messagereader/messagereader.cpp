/****************************************************************************
** Copyright (C) 2010-2019 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

#include "KDSoapMessage.h"
#include "KDSoapMessageReader_p.h"
#include <QTest>
#include <QDebug>

class TestMessageReader : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testLineBreak()
    {
        const QByteArray xmlNoWhitespace =
            "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:dat=\"http://www.27seconds.com/Holidays/US/Dates/\">"
            "<soapenv:Header/>"
            "<soapenv:Body>"
            "<dat:GetEaster>"
            "<dat:year>2011</dat:year>"
            "</dat:GetEaster>"
            "</soapenv:Body>"
            "</soapenv:Envelope>";

        const QByteArray xmlWithWhitespace =
            "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:dat=\"http://www.27seconds.com/Holidays/US/Dates/\">\n"
            "<soapenv:Header/>\n"
            "  <soapenv:Body>\n"
            "    <dat:GetEaster>\n"
            "      <dat:year>2011</dat:year>\n"
            "    </dat:GetEaster>\n"
            "  </soapenv:Body>\n"
            "</soapenv:Envelope>\n";

        const KDSoapMessageReader reader;
        QString ns;
        KDSoapMessage msg;
        KDSoapHeaders headers;
        const KDSoapMessageReader::XmlError err = reader.xmlToMessage(xmlNoWhitespace, &msg, &ns, &headers, KDSoap::SOAP1_1);
        QCOMPARE(err, KDSoapMessageReader::NoError);
        QVERIFY(!msg.isFault());
        QCOMPARE(msg.name(), QLatin1String("GetEaster"));

        const KDSoapMessageReader reader2;
        QString ns2;
        KDSoapMessage msg2;
        KDSoapHeaders headers2;

        const KDSoapMessageReader::XmlError err2 = reader.xmlToMessage(xmlWithWhitespace, &msg2, &ns2, &headers2, KDSoap::SOAP1_1);
        QCOMPARE(err2, KDSoapMessageReader::NoError);
        QCOMPARE(msg2.name(), QLatin1String("GetEaster"));

        if (msg != msg2) {
            QEXPECT_FAIL("", "There is different whitespace in the (unused) value of getEaster", Continue);
            QCOMPARE(msg, msg2);
            qDebug() << msg;
            qDebug() << msg2;
        }
    }

    void testFaultSoap11()
    {
        const QByteArray xmlMissingEnd =
            "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:dat=\"http://www.27seconds.com/Holidays/US/Dates/\">"
            "<soapenv:Header/>"
            "<soapenv:Body>";

        const KDSoapMessageReader reader;
        QString ns;
        KDSoapMessage msg;
        KDSoapHeaders headers;
        const KDSoapMessageReader::XmlError err = reader.xmlToMessage(xmlMissingEnd, &msg, &ns, &headers, KDSoap::SOAP1_1);
        QCOMPARE(err, KDSoapMessageReader::PrematureEndOfDocumentError);
        QVERIFY(msg.isFault());
        QCOMPARE(msg.faultAsString(), QString::fromLatin1(
                     "Fault code 4: XML error: [1:163] Premature end of document."));
    }

    void testFaultSoap12()
    {
        const QByteArray xmlMissingEnd =
            "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:dat=\"http://www.27seconds.com/Holidays/US/Dates/\">"
            "<soapenv:Header/>"
            "<soapenv:Body>";

        const KDSoapMessageReader reader;
        QString ns;
        KDSoapMessage msg;
        KDSoapHeaders headers;
        const KDSoapMessageReader::XmlError err = reader.xmlToMessage(xmlMissingEnd, &msg, &ns, &headers, KDSoap::SOAP1_2);
        QCOMPARE(err, KDSoapMessageReader::PrematureEndOfDocumentError);
        QVERIFY(msg.isFault());
        QCOMPARE(msg.faultAsString(), QString::fromLatin1(
                     "Fault 4: XML error: [1:163] Premature end of document."));
    }
};

QTEST_MAIN(TestMessageReader)

#include "messagereader.moc"

