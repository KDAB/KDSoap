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
#include "KDSoapSocketList_p.h"
#include "KDSoapServerSocket_p.h"
#include "KDSoapServer.h"
#include <QDebug>

KDSoapSocketList::KDSoapSocketList(KDSoapServer* server)
    : m_server(server), m_serverObject(server->createServerObject()), m_totalConnectionCount(0)
{
    Q_ASSERT(m_server);
    Q_ASSERT(m_serverObject);
}

KDSoapSocketList::~KDSoapSocketList()
{
    delete m_serverObject;
}

KDSoapServerSocket* KDSoapSocketList::handleIncomingConnection(int socketDescriptor)
{
    KDSoapServerSocket* socket = new KDSoapServerSocket(this, m_serverObject);
    socket->setSocketDescriptor(socketDescriptor);

#ifndef QT_NO_OPENSSL
    if (m_server->features() & KDSoapServer::Ssl) {
        // We could call a virtual "m_server->setSslConfiguration(socket)" here,
        // if more control is needed (e.g. due to SNI)
        if (!m_server->sslConfiguration().isNull())
            socket->setSslConfiguration(m_server->sslConfiguration());
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
    Q_FOREACH(KDSoapServerSocket* socket, m_sockets)
        socket->close(); // will disconnect
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
