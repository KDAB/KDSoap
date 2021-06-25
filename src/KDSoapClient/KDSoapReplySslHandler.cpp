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

#include <QNetworkReply> //krazy:exclude=includes must come first to define QT_NO_SSL

#ifndef QT_NO_SSL

#include "KDSoapReplySslHandler_p.h"
#include "KDSoapSslHandler.h"

KDSoapReplySslHandler::KDSoapReplySslHandler(QNetworkReply *reply, KDSoapSslHandler *handler)
    : QObject(reply)
    , m_handler(handler)
{
    Q_ASSERT(reply);
    Q_ASSERT(handler);
    QObject::connect(reply, &QNetworkReply::sslErrors, this, &KDSoapReplySslHandler::slotReplySslErrors);
}

void KDSoapReplySslHandler::slotReplySslErrors(const QList<QSslError> &errors)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(parent());
    Q_ASSERT(reply);
    m_handler->handleSslErrors(reply, errors);
}

#endif
