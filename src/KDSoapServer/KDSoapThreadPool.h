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
#ifndef KDSOAPTHREADPOOL_H
#define KDSOAPTHREADPOOL_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include "KDSoapServerGlobal.h"
class KDSoapServer;

/**
 * Pool of threads that can be used to handle SOAP requests in a SOAP server.
 * The thread pool is configured with a maximum number of threads.
 *
 * In case the server application provides different services on different ports,
 * it can decide to use the same thread pool for both services, in order
 * to always respect the maximum number of threads globally.
 */
class KDSOAPSERVER_EXPORT KDSoapThreadPool : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a thread pool with the given \p parent.
     */
    explicit KDSoapThreadPool(QObject *parent = 0);

    /**
     * Destructs the thread pool, after ensuring that all threads finish properly.
     */
    ~KDSoapThreadPool();

    /**
     * Sets the maximum number of threads used by the thread pool.
     * Note: The thread pool will always use at least 1 thread, even if \p maxThreadCount
     * limit is zero or negative.
     * The default maxThreadCount is QThread::idealThreadCount().
     */
    void setMaxThreadCount(int maxThreadCount);

    /**
     * Returns the maximum number of threads used by the thread pool.
     */
    int maxThreadCount() const;

    /**
     * Returns the number of connected sockets for a given server
     */
    int numConnectedSockets(const KDSoapServer *server) const;

    /**
     * Returns the number of sockets that have connected to the given server,
     * in this threadpool, since the last call to resetTotalConnectionCount().
     * \since 1.2
     */
    int totalConnectionCount(const KDSoapServer *server) const;

    /**
     * Resets totalConnectionCount to 0.
     * \since 1.2
     */
    void resetTotalConnectionCount(const KDSoapServer *server);

    /**
     * Disconnect all connected sockets for a given server
     */
    void disconnectSockets(KDSoapServer *server);

private:
    friend class KDSoapServer;
    void handleIncomingConnection(int socketDescriptor, KDSoapServer *server);
    class Private;
    Private *const d;
};

#endif // KDSOAPTHREADPOOL_H
