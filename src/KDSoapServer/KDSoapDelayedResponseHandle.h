/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef KDSOAPDELAYEDRESPONSEHANDLE_H
#define KDSOAPDELAYEDRESPONSEHANDLE_H

#include "KDSoapServerGlobal.h"
#include <QtCore/QSharedDataPointer>

class KDSoapDelayedResponseHandleData;
class KDSoapServerSocket;

/**
 * The delayed-response handle is an opaque data type representing
 * a delayed response. When a server object wants to implement a SOAP method
 * call using an asynchronous operation, it can call KDSoapServerObjectInterface::prepareDelayedResponse(), store
 * the handle, and use the handle later on in order to send the delayed response.
 */
class KDSOAPSERVER_EXPORT KDSoapDelayedResponseHandle
{
public:
    /**
     * Constructs a null handle.
     */
    KDSoapDelayedResponseHandle();
    KDSoapDelayedResponseHandle(const KDSoapDelayedResponseHandle &);
    KDSoapDelayedResponseHandle &operator=(const KDSoapDelayedResponseHandle &);
    ~KDSoapDelayedResponseHandle();

private:
    friend class KDSoapServerObjectInterface;
    explicit KDSoapDelayedResponseHandle(KDSoapServerSocket *socket);
    KDSoapServerSocket *serverSocket() const;
    QSharedDataPointer<KDSoapDelayedResponseHandleData> data;
};

#endif // KDSOAPDELAYEDRESPONSEHANDLE_H
