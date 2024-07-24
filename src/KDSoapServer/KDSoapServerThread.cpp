/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapServer.h"
#include "KDSoapServerSocket_p.h"
#include "KDSoapServerThread_p.h"
#include "KDSoapSocketList_p.h"

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
        // clang-format off
        QMetaObject::invokeMethod(d, "disconnectSocketsForServer", Q_ARG(KDSoapServer*, server), Q_ARG(QSemaphore*, &semaphore));
        // clang-format on
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
    // clang-format off
    QMetaObject::invokeMethod(d, "handleIncomingConnection", Q_ARG(int, socketDescriptor), Q_ARG(KDSoapServer*, server));
    // clang-format on
}

////

KDSoapServerThreadImpl::KDSoapServerThreadImpl()
    : QObject(nullptr)
    , m_incomingConnectionCount(0)
{
}

KDSoapServerThreadImpl::~KDSoapServerThreadImpl()
{
    qDeleteAll(m_socketLists);
}

// Called from main thread!
int KDSoapServerThreadImpl::socketCount()
{
    QMutexLocker lock(&m_socketListMutex);
    int sc = 0;
    for (KDSoapSocketList *socketList : std::as_const(m_socketLists)) {
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
