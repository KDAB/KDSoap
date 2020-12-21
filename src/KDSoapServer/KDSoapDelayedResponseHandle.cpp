/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LicenseRef-KDAB-KDSoap-AGPL3-Modified OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
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
    {}
    // QPointer in case the client disconnects during a delayed response
    QPointer<KDSoapServerSocket> socket;
};

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle()
    : data(new KDSoapDelayedResponseHandleData(nullptr))
{
}

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle(const KDSoapDelayedResponseHandle &rhs) : data(rhs.data)
{
}

KDSoapDelayedResponseHandle &KDSoapDelayedResponseHandle::operator=(const KDSoapDelayedResponseHandle &rhs)
{
    if (this != &rhs) {
        data.operator = (rhs.data);
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
