#include "KDSoapServerThread_p.h"
#include "KDSoapSocketList_p.h"
#include "KDSoapServerSocket_p.h"

KDSoapServerThread::KDSoapServerThread(QObject *parent)
    : QThread(parent), d(0)
{
    qRegisterMetaType<KDSoapServer *>("KDSoapServer*");
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
    d = 0;
}

int KDSoapServerThread::socketCount() const
{
    if (d)
        return d->socketCount();
    return 0;
}

int KDSoapServerThread::socketCountForServer(const KDSoapServer* server) const
{
    if (d)
        return d->socketCountForServer(server);
    return 0;
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
    QMetaObject::invokeMethod(d, "handleIncomingConnection", Q_ARG(int, socketDescriptor), Q_ARG(KDSoapServer*, server));
}

////

KDSoapServerThreadImpl::KDSoapServerThreadImpl()
    : QObject(0), m_incomingConnectionCount(0)
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
    SocketLists::const_iterator it = m_socketLists.constBegin();
    for (; it != m_socketLists.constEnd(); ++it) {
        sc += it.value()->socketCount();
    }
    sc += m_incomingConnectionCount;
    return sc;
}

KDSoapSocketList * KDSoapServerThreadImpl::socketListForServer(KDSoapServer *server)
{
    KDSoapSocketList* sockets = m_socketLists.value(server);
    if (sockets)
        return sockets;

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
    KDSoapSocketList* sockets = socketListForServer(server);
    KDSoapServerSocket* socket = sockets->handleIncomingConnection(socketDescriptor);
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
    KDSoapSocketList* sockets = m_socketLists.value(const_cast<KDSoapServer*>(server));
    return sockets ? sockets->socketCount() : 0;
}
