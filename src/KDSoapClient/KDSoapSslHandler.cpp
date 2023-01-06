/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
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
