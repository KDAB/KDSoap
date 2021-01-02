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
#ifndef KDSOAPSOCKETLIST_P_H
#define KDSOAPSOCKETLIST_P_H

#include <QSet>
#include <QObject>
QT_BEGIN_NAMESPACE
class QTcpSocket;
class QObject;
QT_END_NAMESPACE
class KDSoapServer;
class KDSoapServerSocket;

class KDSoapSocketList : public QObject
{
    Q_OBJECT
public:
    explicit KDSoapSocketList(KDSoapServer *server);
    ~KDSoapSocketList();

    KDSoapServerSocket *handleIncomingConnection(int socketDescriptor);

    int socketCount() const;
    void disconnectAll();

    int totalConnectionCount() const;
    void increaseConnectionCount();
    void resetTotalConnectionCount();

    KDSoapServer *server() const
    {
        return m_server;
    }

public Q_SLOTS:
    void socketDeleted(KDSoapServerSocket *socket);

private:
    KDSoapServer *m_server;
    QObject *m_serverObject;
    QSet<KDSoapServerSocket *> m_sockets;
    QAtomicInt m_totalConnectionCount;
};

#endif // KDSOAPSOCKETLIST_P_H
