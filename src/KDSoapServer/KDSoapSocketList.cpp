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
#include "KDSoapSocketList_p.h"
#include "KDSoapServerSocket_p.h"
#include "KDSoapServer.h"
#include <QDebug>

KDSoapSocketList::KDSoapSocketList(KDSoapServer *server)
    : m_server(server), m_serverObject(server->createServerObject()), m_totalConnectionCount(0)
{
    Q_ASSERT(m_server);
    Q_ASSERT(m_serverObject);
}

KDSoapSocketList::~KDSoapSocketList()
{
    delete m_serverObject;
}

KDSoapServerSocket *KDSoapSocketList::handleIncomingConnection(int socketDescriptor)
{
    KDSoapServerSocket *socket = new KDSoapServerSocket(this, m_serverObject);
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

    QObject::connect(socket, SIGNAL(disconnected()),
                     socket, SLOT(deleteLater()));
    m_sockets.insert(socket);
    connect(socket, SIGNAL(socketDeleted(KDSoapServerSocket*)), this, SLOT(socketDeleted(KDSoapServerSocket*)));
    return socket;
}

void KDSoapSocketList::socketDeleted(KDSoapServerSocket *socket)
{
    //qDebug() << Q_FUNC_INFO;
    m_sockets.remove(socket);
}

int KDSoapSocketList::socketCount() const
{
    return m_sockets.count();
}

void KDSoapSocketList::disconnectAll()
{
    Q_FOREACH (KDSoapServerSocket *socket, m_sockets) {
        socket->close();    // will disconnect
    }
}

int KDSoapSocketList::totalConnectionCount() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    return m_totalConnectionCount.loadAcquire();
#else
    return m_totalConnectionCount;
#endif
}

void KDSoapSocketList::increaseConnectionCount()
{
    m_totalConnectionCount.ref();
    //qDebug() << m_totalConnectionCount << "sockets connected in" << QThread::currentThread();
}

void KDSoapSocketList::resetTotalConnectionCount()
{
    m_totalConnectionCount = 0;
}
