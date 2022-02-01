/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "KDSoapJob.h"
#include "KDSoapMessage.h"

class KDSoapJob::Private
{
public:
    KDSoapHeaders requestHeaders;
    KDSoapMessage reply;
    KDSoapHeaders replyHeaders;
    bool isAutoDelete;
};

KDSoapJob::KDSoapJob(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->isAutoDelete = true;
}

KDSoapJob::~KDSoapJob()
{
    delete d;
}

KDSoapHeaders KDSoapJob::requestHeaders() const
{
    return d->requestHeaders;
}

void KDSoapJob::setRequestHeaders(const KDSoapHeaders &headers)
{
    d->requestHeaders = headers;
}

void KDSoapJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void KDSoapJob::setAutoDelete(bool enable)
{
    d->isAutoDelete = enable;
}

void KDSoapJob::emitFinished(const KDSoapMessage &reply, const KDSoapHeaders &replyHeaders)
{
    d->reply = reply;
    d->replyHeaders = replyHeaders;
    emit finished(this);
    if (d->isAutoDelete) {
        deleteLater();
    }
}

KDSoapMessage KDSoapJob::reply() const
{
    return d->reply;
}

KDSoapHeaders KDSoapJob::replyHeaders() const
{
    return d->replyHeaders;
}

bool KDSoapJob::isFault() const
{
    return d->reply.isFault();
}

QString KDSoapJob::faultAsString() const
{
    if (d->reply.isFault()) {
        return d->reply.faultAsString();
    } else {
        return QString();
    }
}

#include "moc_KDSoapJob.cpp"
