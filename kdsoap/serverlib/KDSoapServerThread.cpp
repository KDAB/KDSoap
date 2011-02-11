#include "KDSoapServerThread_p.h"
#include "KDSoapSocketList_p.h"
#include "KDSoapServerSocket_p.h"

KDSoapServerThread::KDSoapServerThread(QObject *parent)
    : QThread(parent)
{
}

// Called from main thread!
int KDSoapServerThread::socketCount() const
{
    int sc = 0;
    SocketLists::const_iterator it = m_socketLists.constBegin();
    for (; it != m_socketLists.constEnd(); ++it) {
        sc += it.value()->socketCount();
    }
    return sc;
}

// Called from main thread!
KDSoapSocketList * KDSoapServerThread::socketListForServer(KDSoapServer *server)
{
    KDSoapSocketList* sockets = m_socketLists.value(server);
    if (sockets)
        return sockets;

    sockets = new KDSoapSocketList(server);
    return sockets;
}

// Called from main thread!
void KDSoapServerThread::handleIncomingConnection(int socketDescriptor, KDSoapServer *server)
{
    KDSoapSocketList* sockets = socketListForServer(server);
    KDSoapServerSocket* socket = sockets->handleIncomingConnection(socketDescriptor);
    socket->moveToThread(this); // push to the given thread (looks like a pull, but it's not)
}
