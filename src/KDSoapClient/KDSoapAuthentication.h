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
#ifndef KDSOAPAUTHENTICATION_H
#define KDSOAPAUTHENTICATION_H

#include "KDSoapGlobal.h"
#include <QtCore/QUrl>
QT_BEGIN_NAMESPACE
class QAuthenticator;
class QDateTime;
class QNetworkReply;
class QXmlStreamWriter;
QT_END_NAMESPACE
class KDSoapNamespacePrefixes;

/**
 * KDSoapAuthentication provides an authentication object.
 * Currently it only supports authentication based on user/password,
 * but its design makes it extensible to other forms of authentication.
 *
 * \see KDSoapClientInterface::setAuthentication()
 */
class KDSOAP_EXPORT KDSoapAuthentication
{
public:
    friend class KDSoapMessageWriter;
    friend class KDSoapClientInterfacePrivate;
    friend class KDSoapThreadTask;

    /**
     * Constructs an empty authentication object.
     */
    KDSoapAuthentication();
    /**
     * Constructs a copy of \p other.
     */
    KDSoapAuthentication(const KDSoapAuthentication &other);
    /**
     * Destructs the object
     */
    ~KDSoapAuthentication();

    /**
     * Sets the \p user used for authentication
     */
    void setUser(const QString &user);
    /**
     * \return the user used for authentication
     */
    QString user() const;

    /**
     * Sets the \p password used for authentication
     */
    void setPassword(const QString &password);
    /**
     * \return the password used for authentication
     */
    QString password() const;

    /**
     * Sets whether WS-UsernameToken is used for authentication. When
     * set, the WS-UsernameToken headers are included in each request.
     * \since 1.8
     */
    void setUseWSUsernameToken(bool useWSUsernameToken);
    /**
     * \since 1.8
     * \return whether WS-UsernameToken is used for authentication
     */
    bool useWSUsernameToken() const;

    /**
     * Sets the created time used during WS-UsernameToken authentication
     * This is useful for devices with an incorrect time and during testing
     * \since 1.8
     */
    void setOverrideWSUsernameCreatedTime(QDateTime overrideWSUsernameCreatedTime);
    /**
     * \since 1.8
     * \return the created time used during WS-UsernameToken authentication
     */
    QDateTime overrideWSUsernameCreatedTime() const;

    /**
     * Sets the nonce used during WS-UsernameToken authentication
     * This is useful during testing
     * \since 1.8
     */
    void setOverrideWSUsernameNonce(QByteArray overrideWSUsernameNonce);
    /**
     * \since 1.8
     * \return the created time used during WS-UsernameToken authentication
     */
    QByteArray overrideWSUsernameNonce() const;

    /**
     * \return \c true if authentication was defined, or
     * \c false if this object is only a default-constructed KDSoapAuthentication().
     */
    bool hasAuth() const;

    /**
     * Assigns the contents of \p other to this authenticator.
     */
    KDSoapAuthentication &operator=(const KDSoapAuthentication &other);

private:
    /**
     * \internal
     */
    void handleAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

    bool hasWSUsernameTokenHeader() const;

    void writeWSUsernameTokenHeader(KDSoapNamespacePrefixes &namespacePrefixes, QXmlStreamWriter &writer, const QString &messageNamespace, bool forceQualified) const;

private:
    class Private;
    Private *const d;
};

#endif // KDSOAPAUTHENTICATION_H
