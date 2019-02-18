/****************************************************************************
** Copyright (C) 2010-2018 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** Copyright (C) 2019 Casper Meijn  <casper@meijn.net>
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

#include "httpserver_p.h"

#include <QtTest/QtTest>
#include <QObject>

#include "KDSoapNamespaceManager.h"
#include "KDSoapAuthentication.h"
#include "wsdl_wsusernametoken.h"

using namespace KDSoapUnitTestHelpers;

class WSUsernameTokenTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void shouldWriteAProperSoapMessageWithRightUsernameToken()
    {
        // GIVEN
        HttpServerThread server(emptyResponse(), HttpServerThread::Public);
        KDSoapClientInterface client(server.endPoint(), "http://www.ecerami.com/wsdl/HelloService");

        KDSoapAuthentication auth;
        auth.setUser("admin");
        auth.setPassword("userpassword");
        auth.setUseWSUsernameToken(true);

        // Override the nonce and timestamp to make the test output consistent.
        auth.setOverrideWSUsernameNonce(QByteArray::fromBase64("LKqI6G/AikKCQrN0zqZFlg=="));
        auth.setOverrideWSUsernameCreatedTime(QDateTime(QDate(2010, 9, 16), QTime(7, 50, 45), Qt::UTC));//2010-09-16T07:50:45Z

        client.setAuthentication(auth);

        // make a request
        KDSoapMessage message;
        const QString action = QString::fromLatin1("sayHello");
        message.setUse(KDSoapMessage::EncodedUse);
        message.addArgument(QString::fromLatin1("msg"), QVariant::fromValue(QString("HelloContentMessage")), KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("string"));
        message.setNamespaceUri(QString::fromLatin1("http://www.ecerami.com/wsdl/HelloService.wsdl"));

        // WHEN
        KDSoapMessage reply = client.call(QLatin1String("sayHello"), message, action);

        // THEN
        QVERIFY(xmlBufferCompare(server.receivedData(), expectedSoapMessage()));
    }

private:
    static QByteArray expectedSoapMessage()
    {
        return QByteArray(xmlEnvBegin11()) +
                " xmlns:n1=\"http://www.ecerami.com/wsdl/HelloService.wsdl\">"
                "  <soap:Header> "
                "    <n2:Security xmlns:n2=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"> "
                "      <n2:UsernameToken> "
                "        <n2:Nonce>LKqI6G/AikKCQrN0zqZFlg==</n2:Nonce> "
                "        <n3:Created xmlns:n3=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">2010-09-16T07:50:45Z</n3:Created> "
                "        <n2:Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">tuOSpGlFlIXsozq4HFNeeGeFLEI=</n2:Password> "
                "        <n2:Username>admin</n2:Username> "
                "      </n2:UsernameToken> "
                "    </n2:Security> "
                "  </soap:Header> "
                "  <soap:Body> "
                "    <n1:sayHello><msg xsi:type=\"xsd:string\">HelloContentMessage</msg></n1:sayHello> "
                "  </soap:Body> " + xmlEnvEnd();
    }

    static QByteArray emptyResponse()
    {
        return QByteArray(xmlEnvBegin11()) + "><soap:Body/>";
    }
};

QTEST_MAIN(WSUsernameTokenTest)

#include "wsusernametokentest.moc"
