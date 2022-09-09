/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
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
