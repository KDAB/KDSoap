/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapDelayedResponseHandle.h"
#include "KDSoapServerSocket_p.h"
#include <QSharedData>
#include <QPointer>

class KDSoapDelayedResponseHandleData : public QSharedData
{
public:
    KDSoapDelayedResponseHandleData(KDSoapServerSocket *s)
        : socket(s)
    {
    }
    // QPointer in case the client disconnects during a delayed response
    QPointer<KDSoapServerSocket> socket;
};

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle()
    : data(new KDSoapDelayedResponseHandleData(nullptr))
{
}

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle(const KDSoapDelayedResponseHandle &rhs)
    : data(rhs.data)
{
}

KDSoapDelayedResponseHandle &KDSoapDelayedResponseHandle::operator=(const KDSoapDelayedResponseHandle &rhs)
{
    if (this != &rhs) {
        data.operator=(rhs.data);
    }
    return *this;
}

KDSoapDelayedResponseHandle::~KDSoapDelayedResponseHandle()
{
}

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle(KDSoapServerSocket *socket)
    : data(new KDSoapDelayedResponseHandleData(socket))
{
    socket->setResponseDelayed();
}

KDSoapServerSocket *KDSoapDelayedResponseHandle::serverSocket() const
{
    return data->socket;
}
