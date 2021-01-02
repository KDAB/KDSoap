/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
