/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapPendingCallWatcher_p.h"
#include "KDSoapPendingCall_p.h"
#include <QDebug>
#include <QNetworkReply>

KDSoapPendingCallWatcher::KDSoapPendingCallWatcher(const KDSoapPendingCall &call, QObject *parent)
    : QObject(parent)
    , KDSoapPendingCall(call)
    , d(nullptr) // currently unused
{
    connect(call.d->reply.data(), &QNetworkReply::finished, this, [&]() {
        emit finished(this);
    });
}

KDSoapPendingCallWatcher::~KDSoapPendingCallWatcher()
{
}

#include "moc_KDSoapPendingCallWatcher.cpp"
