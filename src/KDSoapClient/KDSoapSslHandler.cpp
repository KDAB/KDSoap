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

#include <QNetworkReply> //may define QT_NO_SSL. krazy:exclude=includes

#ifndef QT_NO_SSL
#include "KDSoapSslHandler.h"

KDSoapSslHandler::KDSoapSslHandler(QObject *parent)
    : QObject(parent)
    , m_reply(nullptr)
{
}

KDSoapSslHandler::~KDSoapSslHandler()
{
}

void KDSoapSslHandler::ignoreSslErrors()
{
    Q_ASSERT(m_reply);
    m_reply->ignoreSslErrors();
}

void KDSoapSslHandler::ignoreSslErrors(const QList<QSslError> &errors)
{
    Q_ASSERT(m_reply);
    m_reply->ignoreSslErrors(errors);
}

void KDSoapSslHandler::handleSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    m_reply = reply;
    Q_ASSERT(m_reply);
    Q_EMIT sslErrors(this, errors);
}
#endif
