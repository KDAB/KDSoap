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
#include "KDSoapServerThread_p.h"
#include "KDSoapSocketList_p.h"
#include "KDSoapServerSocket_p.h"
#include "KDSoapServer.h"

#include <QMetaType>

KDSoapServerThread::KDSoapServerThread(QObject *parent)
    : QThread(parent)
    , d(nullptr)
{
    qRegisterMetaType<KDSoapServer *>("KDSoapServer*");
    qRegisterMetaType<QSemaphore *>("QSemaphore*");
}

KDSoapServerThread::~KDSoapServerThread()
{
}

void KDSoapServerThread::run()
{
    KDSoapServerThreadImpl impl;
    d = &impl;
    m_semaphore.release();
    exec();
    d = nullptr;
}

int KDSoapServerThread::socketCount() const
{
    if (d) {
        return d->socketCount();
    }
    return 0;
}

int KDSoapServerThread::socketCountForServer(const KDSoapServer *server) const
{
    if (d) {
        return d->socketCountForServer(server);
    }
    return 0;
}

int KDSoapServerThread::totalConnectionCountForServer(const KDSoapServer *server) const
{
    if (d) {
        return d->totalConnectionCountForServer(server);
    }
    return 0;
}

void KDSoapServerThread::resetTotalConnectionCountForServer(const KDSoapServer *server)
{
    if (d) {
        d->resetTotalConnectionCountForServer(server);
    }
}

void KDSoapServerThread::disconnectSocketsForServer(KDSoapServer *server, QSemaphore &semaphore)
{
    if (d) {
        QMetaObject::invokeMethod(d, "disconnectSocketsForServer", Q_ARG(KDSoapServer *, server), Q_ARG(QSemaphore *, &semaphore));
    }
}

void KDSoapServerThread::startThread()
{
    QThread::start();
    m_semaphore.acquire(); // wait for init to be done
}

void KDSoapServerThread::quitThread()
{
    QMetaObject::invokeMethod(d, "quit");
}

void KDSoapServerThread::handleIncomingConnection(int socketDescriptor, KDSoapServer *server)
{
    d->addIncomingConnection();
    QMetaObject::invokeMethod(d, "handleIncomingConnection", Q_ARG(int, socketDescriptor), Q_ARG(KDSoapServer *, server));
}

////

KDSoapServerThreadImpl::KDSoapServerThreadImpl()
    : QObject(nullptr)
    , m_incomingConnectionCount(0)
{
}

KDSoapServerThreadImpl::~KDSoapServerThreadImpl()
{
    qDeleteAll(m_socketLists.values());
}

// Called from main thread!
int KDSoapServerThreadImpl::socketCount()
{
    QMutexLocker lock(&m_socketListMutex);
    int sc = 0;
    for (KDSoapSocketList *socketList : qAsConst(m_socketLists)) {
        sc += socketList->socketCount();
    }
    sc += m_incomingConnectionCount.loadAcquire();
    return sc;
}

KDSoapSocketList *KDSoapServerThreadImpl::socketListForServer(KDSoapServer *server)
{
    KDSoapSocketList *sockets = m_socketLists.value(server);
    if (sockets) {
        return sockets;
    }

    sockets = new KDSoapSocketList(server); // creates the server object
    m_socketLists.insert(server, sockets);
    return sockets;
}

void KDSoapServerThreadImpl::addIncomingConnection()
{
    m_incomingConnectionCount.fetchAndAddAcquire(1);
}

// Called in the thread itself so that the socket list and server object
// are created in the thread.
void KDSoapServerThreadImpl::handleIncomingConnection(int socketDescriptor, KDSoapServer *server)
{
    QMutexLocker lock(&m_socketListMutex);
    KDSoapSocketList *sockets = socketListForServer(server);
    KDSoapServerSocket *socket = sockets->handleIncomingConnection(socketDescriptor);
    Q_UNUSED(socket);
    m_incomingConnectionCount.fetchAndAddAcquire(-1);
}

void KDSoapServerThreadImpl::quit()
{
    thread()->quit();
}

int KDSoapServerThreadImpl::socketCountForServer(const KDSoapServer *server)
{
    QMutexLocker lock(&m_socketListMutex);
    KDSoapSocketList *sockets = m_socketLists.value(const_cast<KDSoapServer *>(server));
    return sockets ? sockets->socketCount() : 0;
}

void KDSoapServerThreadImpl::disconnectSocketsForServer(KDSoapServer *server, QSemaphore *semaphore)
{
    QMutexLocker lock(&m_socketListMutex);
    KDSoapSocketList *sockets = m_socketLists.value(server);
    if (sockets) {
        sockets->disconnectAll();
    }
    semaphore->release();
}

int KDSoapServerThreadImpl::totalConnectionCountForServer(const KDSoapServer *server)
{
    QMutexLocker lock(&m_socketListMutex);
    KDSoapSocketList *sockets = m_socketLists.value(const_cast<KDSoapServer *>(server));
    return sockets ? sockets->totalConnectionCount() : 0;
}

void KDSoapServerThreadImpl::resetTotalConnectionCountForServer(const KDSoapServer *server)
{
    QMutexLocker lock(&m_socketListMutex);
    KDSoapSocketList *sockets = m_socketLists.value(const_cast<KDSoapServer *>(server));
    if (sockets) {
        sockets->resetTotalConnectionCount();
    }
}
