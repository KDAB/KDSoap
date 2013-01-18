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

#include "KDSoapJob.h"
#include "KDSoapMessage.h"

class KDSoapJob::Private {
public:
    KDSoapMessage reply;
    KDSoapHeaders replyHeaders;
};


KDSoapJob::KDSoapJob(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

KDSoapJob::~KDSoapJob()
{
    delete d;
}

void KDSoapJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void KDSoapJob::emitFinished(const KDSoapMessage &reply, const KDSoapHeaders &replyHeaders)
{
    d->reply = reply;
    d->replyHeaders = replyHeaders;
    emit finished(this);
    deleteLater();
}

KDSoapMessage KDSoapJob::reply() const
{
    return d->reply;
}

bool KDSoapJob::isFault() const
{
    return d->reply.isFault();
}

QString KDSoapJob::faultAsString() const
{
    if (d->reply.isFault())
        return d->reply.faultAsString();
    else
        return QString();
}

#include "moc_KDSoapJob.cpp"
