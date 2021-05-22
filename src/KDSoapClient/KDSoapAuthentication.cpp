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
#include "KDSoapAuthentication.h"
#include <QAuthenticator>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QNetworkReply>
#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapNamespaceManager.h"
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif

class KDSoapAuthentication::Private
{
public:
    QString user;
    QString password;
    bool usePasswordDigest = false;
    bool useWSUsernameToken = false;
    QDateTime overrideWSUsernameCreatedTime;
    QByteArray overrideWSUsernameNonce;
};

KDSoapAuthentication::KDSoapAuthentication()
    : d(new Private)
{
    d->usePasswordDigest = true;
}

KDSoapAuthentication::KDSoapAuthentication(const KDSoapAuthentication &other)
    : d(new Private)
{
    *d = *other.d;
}

KDSoapAuthentication &KDSoapAuthentication::operator=(const KDSoapAuthentication &other)
{
    *d = *other.d;
    return *this;
}

KDSoapAuthentication::~KDSoapAuthentication()
{
    delete d;
}

void KDSoapAuthentication::setUser(const QString &user)
{
    d->user = user;
}

void KDSoapAuthentication::setPassword(const QString &password)
{
    d->password = password;
}

void KDSoapAuthentication::setUsePasswordDigest(const bool usePasswordDigest)
{
    d->usePasswordDigest = usePasswordDigest;
}

void KDSoapAuthentication::setUseWSUsernameToken(bool useWSUsernameToken)
{
    d->useWSUsernameToken = useWSUsernameToken;
}

void KDSoapAuthentication::setOverrideWSUsernameCreatedTime(QDateTime overrideWSUsernameCreatedTime)
{
    d->overrideWSUsernameCreatedTime = overrideWSUsernameCreatedTime;
}

void KDSoapAuthentication::setOverrideWSUsernameNonce(QByteArray overrideWSUsernameNonce)
{
    d->overrideWSUsernameNonce = overrideWSUsernameNonce;
}

QString KDSoapAuthentication::user() const
{
    return d->user;
}

QString KDSoapAuthentication::password() const
{
    return d->password;
}

bool KDSoapAuthentication::usePasswordDigest() const
{
    return d->usePasswordDigest;
}

bool KDSoapAuthentication::useWSUsernameToken() const
{
    return d->useWSUsernameToken;
}

QDateTime KDSoapAuthentication::overrideWSUsernameCreatedTime() const
{
    return d->overrideWSUsernameCreatedTime;
}

QByteArray KDSoapAuthentication::overrideWSUsernameNonce() const
{
    return d->overrideWSUsernameNonce;
}

bool KDSoapAuthentication::hasAuth() const
{
    return !d->user.isEmpty() || !d->password.isEmpty();
}

void KDSoapAuthentication::handleAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    // qDebug() << "handleAuthenticationRequired" << reply << reply->url() << "realm=" << authenticator->realm();
    // Only proceed if
    // 1) we have some authentication to offer
    // 2) we didn't try once already (unittest: BuiltinHttpTest::testAsyncCallRefusedAuth)
    if (hasAuth() && !reply->property("authAdded").toBool()) {
        authenticator->setUser(d->user);
        authenticator->setPassword(d->password);
        reply->setProperty("authAdded", true);
    }
}

bool KDSoapAuthentication::hasWSUsernameTokenHeader() const
{
    return hasAuth() && d->useWSUsernameToken;
}

void KDSoapAuthentication::writeWSUsernameTokenHeader(QXmlStreamWriter &writer) const
{
    if (!hasAuth()) {
        return;
    }

    const QString securityExtentionNS = KDSoapNamespaceManager::soapSecurityExtention();
    const QString securityUtilityNS = KDSoapNamespaceManager::soapSecurityUtility();
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    static QRandomGenerator generator;
    QByteArray nonce = "kdsoap" + QByteArray::number(generator.generate64());
#else
    QByteArray nonce = "kdsoap" + QByteArray::number(qrand());
#endif
    if (!d->overrideWSUsernameNonce.isEmpty()) {
        nonce = d->overrideWSUsernameNonce;
    }
    QDateTime time = QDateTime::currentDateTimeUtc();
    if (d->overrideWSUsernameCreatedTime.isValid()) {
        time = d->overrideWSUsernameCreatedTime;
    }
    QString timestamp = time.toString(QLatin1String("yyyy-MM-ddTHH:mm:ssZ"));

    writer.writeStartElement(securityExtentionNS, QLatin1String("Security"));
    writer.writeStartElement(securityExtentionNS, QLatin1String("UsernameToken"));

    writer.writeStartElement(securityExtentionNS, QLatin1String("Nonce"));
    writer.writeCharacters(QString::fromLatin1(nonce.toBase64().constData()));
    writer.writeEndElement();

    writer.writeStartElement(securityUtilityNS, QLatin1String("Created"));
    writer.writeCharacters(timestamp);
    writer.writeEndElement();

    writer.writeStartElement(securityExtentionNS, QLatin1String("Password"));
    if (d->usePasswordDigest) {
        writer.writeAttribute(QLatin1String("Type"),
                              QLatin1String("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest"));
        QByteArray passwordConcat = nonce + timestamp.toUtf8() + d->password.toUtf8();
        QByteArray passwordHash = QCryptographicHash::hash(passwordConcat, QCryptographicHash::Sha1);
        writer.writeCharacters(QString::fromLatin1(passwordHash.toBase64().constData()));
    } else {
        writer.writeAttribute(QLatin1String("Type"),
                              QLatin1String("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordText"));
        writer.writeCharacters(d->password);
    }
    writer.writeEndElement();

    writer.writeStartElement(securityExtentionNS, QLatin1String("Username"));
    writer.writeCharacters(d->user);
    writer.writeEndElement();

    writer.writeEndElement();
    writer.writeEndElement();
}
