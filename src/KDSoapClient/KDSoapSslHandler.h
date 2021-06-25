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

#ifndef KDSOAPSSLHANDLER_H
#define KDSOAPSSLHANDLER_H

#include <QObject>
#include <QSslError>
#include "KDSoapGlobal.h"

QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

#ifndef QT_NO_SSL

class KDSoapReplySslHandler;
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
     * @param handler is a pointer the KDSoapSSLHandler associated with the errors
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

    /**
     * If this function is called, the SSL errors given in \p errors will be ignored.
     * Note that you can set the expected certificate in the SSL error.
     * See QNetworkReply::ignoreSslErrors() for more information.
     *
     * This function can be called from the slot connected to the sslErrors() signal, which
     * indicates which errors were found.
     *
     * \param errors list of errors to ignore
     * \since 1.4
     */
    void ignoreSslErrors(const QList<QSslError> &errors);

private:
    friend class KDSoapReplySslHandler;
    void handleSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    friend class KDSoapClientInterface;
    friend class KDSoapClientInterfacePrivate;
    /**
     * Constructs a KDSoapSslHandler.
     * Used internally
     */
    explicit KDSoapSslHandler(QObject *parent = nullptr);
    virtual ~KDSoapSslHandler();

    QNetworkReply *m_reply;
};

#endif // QT_NO_SSL

#endif // KDSOAPSSLHANDLER_H
