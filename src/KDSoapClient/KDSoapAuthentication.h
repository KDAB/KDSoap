/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
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
class QNetworkReply;
QT_END_NAMESPACE

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
    /**
     * Constructs an empty authentication object.
     */
    KDSoapAuthentication();
    /**
     * Constructs a copy of \p other.
     */
    KDSoapAuthentication(const KDSoapAuthentication& other);
    /**
     * Destructs the object
     */
    ~KDSoapAuthentication();

    /**
     * Sets the \p user used for authentication
     */
    void setUser(const QString& user);
    /**
     * \return the user used for authentication
     */
    QString user() const;

    /**
     * Sets the \p password used for authentication
     */
    void setPassword(const QString& password);
    /**
     * \return the password used for authentication
     */
    QString password() const;

    /**
     * \return \c true if authentication was defined, or
     * \c false if this object is only a default-constructed KDSoapAuthentication().
     */
    bool hasAuth() const;

    /**
     * Assigns the contents of \p other to this authenticator.
     */
    KDSoapAuthentication& operator=(const KDSoapAuthentication& other);

    /**
     * \internal
     */
    void handleAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);

private:
    class Private;
    Private * const d;
};

#endif // KDSOAPAUTHENTICATION_H
