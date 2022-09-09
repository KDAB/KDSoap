/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
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
