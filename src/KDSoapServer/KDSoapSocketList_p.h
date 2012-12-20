/****************************************************************************
** Copyright (C) 2010-2012 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
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
    explicit KDSoapSocketList(KDSoapServer* server);
    ~KDSoapSocketList();

    KDSoapServerSocket* handleIncomingConnection(int socketDescriptor);

    int socketCount() const;
    void disconnectAll();

    int totalConnectionCount() const;
    void resetTotalConnectionCount();

    KDSoapServer* server() const { return m_server; }

public Q_SLOTS:
    void socketDeleted(KDSoapServerSocket* socket);

private:
    KDSoapServer* m_server;
    QObject* m_serverObject;
    QSet<KDSoapServerSocket *> m_sockets;
    QAtomicInt m_totalConnectionCount;
};

#endif // KDSOAPSOCKETLIST_P_H
