/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapServerAuthInterface.h"
#include <KDSoapClient/KDSoapAuthentication.h>

KDSoapServerAuthInterface::KDSoapServerAuthInterface()
    : d(nullptr)
{
}

KDSoapServerAuthInterface::~KDSoapServerAuthInterface()
{
}

enum Method
{
    None,
    Basic,
    Plain,
    Login,
    Ntlm,
    CramMd5,
    DigestMd5
};
static void parseAuthLine(const QString &str, Method *method, QString *headerVal)
{
    *method = None;
    // The code below (from QAuthenticatorPrivate::parseHttpResponse)
    // is supposed to be run in a loop, apparently
    // (multiple WWW-Authenticate lines? multiple values in the line?)

    // qDebug() << "parseAuthLine() " << str;
    if (*method < Basic && str.startsWith(QLatin1String("Basic"), Qt::CaseInsensitive)) {
        *method = Basic;
        *headerVal = str.mid(6);
    } else if (*method < Ntlm && str.startsWith(QLatin1String("NTLM"), Qt::CaseInsensitive)) {
        *method = Ntlm;
        *headerVal = str.mid(5);
    } else if (*method < DigestMd5 && str.startsWith(QLatin1String("Digest"), Qt::CaseInsensitive)) {
        *method = DigestMd5;
        *headerVal = str.mid(7);
    }
}

bool KDSoapServerAuthInterface::handleHttpAuth(const QByteArray &authValue, const QString &path)
{
    bool authOk = false;
    KDSoapAuthentication authSettings;
    if (authValue.isEmpty()) {
        // Let the implementation decide whether it accepts "no auth".
        authOk = validateAuthentication(authSettings, path);
    } else {
        // qDebug() << "got authValue=" << authValue; // looks like "Basic <base64 of user:pass>"
        Method method;
        QString headerVal;
        parseAuthLine(QString::fromLatin1(authValue.constData(), authValue.size()), &method, &headerVal);
        // qDebug() << "method=" << method << "headerVal=" << headerVal;
        switch (method) {
        case None:
            // Let the implementation decide whether it accepts "no auth".
            authOk = validateAuthentication(authSettings, path);
            break;
        case Basic: {
            const QByteArray userPass = QByteArray::fromBase64(headerVal.toLatin1());
            const int separatorPos = userPass.indexOf(':');
            if (separatorPos == -1) {
                break;
            }
            authSettings.setUser(QString::fromUtf8(userPass.left(separatorPos).constData()));
            authSettings.setPassword(QString::fromUtf8(userPass.mid(separatorPos + 1).constData()));
            authOk = validateAuthentication(authSettings, path);
            break;
        }
        default:
            qWarning("Unsupported authentication mechanism %s", authValue.constData());
            break;
        }
    }
    return authOk;
}

bool KDSoapServerAuthInterface::validateAuthentication(const KDSoapAuthentication &auth, const QString &path)
{
    Q_UNUSED(auth);
    Q_UNUSED(path);
    return false;
}
