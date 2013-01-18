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

#ifndef KDSOAPSSLHANDLER_H
#define KDSOAPSSLHANDLER_H

#include <QObject>
#include <QSslError>
#include "KDSoapGlobal.h"

QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

#ifndef QT_NO_OPENSSL
/**
 * \brief A class for handling SSL errors during SOAP calls
 *
 * \since 1.3
 */
class KDSOAP_EXPORT KDSoapSslHandler : public QObject
{
    Q_OBJECT
public:

Q_SIGNALS:
    /**
     * @brief Notification of SSL errors
     *
     * This signal is emitted if the SSL/TLS session encountered errors during the set
     * up, including certificate verification errors. The errors parameter contains the list of errors.
     *
     * To indicate that the errors are not fatal and that the connection should proceed, the ignoreSslErrors()
     * method should be called from the slot connected to this signal. If it is not called, the SSL session
     * will be torn down before any data is exchanged.
     *
     * @param errors list of ssl errors
     */
    void sslErrors(KDSoapSslHandler *handler, const QList<QSslError> &errors);

public Q_SLOTS:
    /**
     * @brief ignoreSslErrors
     *
     * If this function is called, SSL errors related to network connection will be ignored,
     * including certificate validation errors.
     *
     * Note that calling this function without restraint may pose a security risk for your application.
     * Use it with care.
     *
     * This function can be called from the slot connected to the sslErrors() signal, which
     * indicates which errors were found.
     */
    void ignoreSslErrors();

private Q_SLOTS:
    void slotSslErrors(const QList<QSslError> &errors);

private:
    friend class KDSoapClientInterface;
    friend class KDSoapClientInterfacePrivate;
    /**
     * Constructs a KDSoapSslHandler.
     * Used internally
     */
    explicit KDSoapSslHandler(QObject *parent = 0);
    virtual ~KDSoapSslHandler();

    QNetworkReply* m_reply; // could be replaced with a d pointer if needed
};

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(KDSoapSslHandler *)
#endif

#endif // QT_NO_OPENSSL

#endif // KDSOAPSSLHANDLER_H
