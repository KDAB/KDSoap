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
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapPendingCallWatcher_p.h"
#include "KDSoapPendingCall_p.h"
#include <QNetworkReply>
#include <QDebug>

KDSoapPendingCallWatcher::KDSoapPendingCallWatcher(const KDSoapPendingCall &call, QObject *parent)
    : QObject(parent), KDSoapPendingCall(call),
    d(new Private(this))
{
    connect(call.d->reply.data(), SIGNAL(finished()), this, SLOT(_kd_slotReplyFinished()));
}

KDSoapPendingCallWatcher::~KDSoapPendingCallWatcher()
{
    delete d;
}

#if 0
void KDSoapPendingCallWatcher::waitForFinished()
{
    // TODO?
}
#endif

void KDSoapPendingCallWatcher::Private::_kd_slotReplyFinished()
{
    // Workaround Qt-4.5 emitting finished twice in testCallRefusedAuth
    disconnect(q->KDSoapPendingCall::d->reply.data(), SIGNAL(finished()), q, 0);
    emit q->finished(q);
}

#include "moc_KDSoapPendingCallWatcher.cpp"
