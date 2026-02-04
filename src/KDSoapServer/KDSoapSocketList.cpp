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
#include "KDSoapSocketList_p.h"
#include <QDebug>

KDSoapSocketList::KDSoapSocketList(KDSoapServer *server)
    : m_server(server)
    , m_totalConnectionCount(0)
{
    Q_ASSERT(m_server);
}

KDSoapSocketList::~KDSoapSocketList()
{
    clear();
}

KDSoapServerSocket *KDSoapSocketList::handleIncomingConnection(int socketDescriptor)
{
    KDSoapServerSocket *socket = new KDSoapServerSocket(this, m_server->createServerObject());
    socket->setSocketDescriptor(socketDescriptor);

#ifndef QT_NO_SSL
    if (m_server->features() & KDSoapServer::Ssl) {
        // We could call a virtual "m_server->setSslConfiguration(socket)" here,
        // if more control is needed (e.g. due to SNI)
        if (!m_server->sslConfiguration().isNull()) {
            socket->setSslConfiguration(m_server->sslConfiguration());
        }
        socket->startServerEncryption();
    }
#endif

    QObject::connect(socket, &KDSoapServerSocket::disconnected, this, [socket, this]() {
        if (!socket->disconnectedByServer()) {
            m_sockets.remove(socket);
            socket->deleteLater();
        }
    });
    m_sockets.insert(socket);
    return socket;
}

int KDSoapSocketList::socketCount() const
{
    return m_sockets.count();
}

void KDSoapSocketList::clear()
{
    for (KDSoapServerSocket *socket : std::as_const(m_sockets)) {
        socket->disconnectSocket();
        socket->deleteLater();
    }
    m_sockets.clear();
}

int KDSoapSocketList::totalConnectionCount() const
{
    return m_totalConnectionCount.loadAcquire();
}

void KDSoapSocketList::increaseConnectionCount()
{
    m_totalConnectionCount.ref();
    // qDebug() << m_totalConnectionCount << "sockets connected in" << QThread::currentThread();
}

void KDSoapSocketList::resetTotalConnectionCount()
{
    m_totalConnectionCount = 0;
}

#include "moc_KDSoapSocketList_p.cpp"
