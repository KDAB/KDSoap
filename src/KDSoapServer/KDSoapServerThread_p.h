/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPSERVERTHREAD_P_H
#define KDSOAPSERVERTHREAD_P_H

#include <QHash>
#include <QMutex>
#include <QSemaphore>
#include <QThread>
class KDSoapServer;
class KDSoapSocketList;

// clazy:excludeall=ctor-missing-parent-argument
class KDSoapServerThreadImpl : public QObject
{
    Q_OBJECT
public:
    KDSoapServerThreadImpl(); // created on stack, clazy:exclude=ctor-missing-parent-argument
    ~KDSoapServerThreadImpl();

public Q_SLOTS:
    void handleIncomingConnection(int socketDescriptor, KDSoapServer *server);
    void disconnectSocketsForServer(KDSoapServer *server, QSemaphore *semaphore);
    void quit();

public:
    int socketCount();
    int socketCountForServer(const KDSoapServer *server);
    int totalConnectionCountForServer(const KDSoapServer *server);
    void resetTotalConnectionCountForServer(const KDSoapServer *server);

    void addIncomingConnection();

private:
    QMutex m_socketListMutex;
    KDSoapSocketList *socketListForServer(KDSoapServer *server);
    typedef QHash<KDSoapServer *, KDSoapSocketList *> SocketLists;
    SocketLists m_socketLists;

    QAtomicInt m_incomingConnectionCount;
};

class KDSoapServerThread : public QThread
{
    Q_OBJECT
public:
    explicit KDSoapServerThread(QObject *parent = 0);
    ~KDSoapServerThread();

    void startThread();
    void quitThread();

    int socketCount() const;
    int socketCountForServer(const KDSoapServer *server) const;
    int totalConnectionCountForServer(const KDSoapServer *server) const;
    void resetTotalConnectionCountForServer(const KDSoapServer *server);

    void disconnectSocketsForServer(KDSoapServer *server, QSemaphore &semaphore);
    void handleIncomingConnection(int socketDescriptor, KDSoapServer *server);

protected:
    virtual void run() override;

private:
    void start(); // use startThread instead
    void quit(); // use quitThread instead
    KDSoapServerThreadImpl *d;
    QSemaphore m_semaphore;
};

#endif // KDSOAPSERVERTHREAD_P_H
