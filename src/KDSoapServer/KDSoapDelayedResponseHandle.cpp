/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

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

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle() : data(new KDSoapDelayedResponseHandleData(0))
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
