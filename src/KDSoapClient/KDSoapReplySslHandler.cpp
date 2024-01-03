/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
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
