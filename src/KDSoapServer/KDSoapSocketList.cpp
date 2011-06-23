#include "KDSoapSocketList_p.h"
#include "KDSoapServerSocket_p.h"
#include "KDSoapServer.h"
#include <QDebug>

const bool doDebug = true;

KDSoapSocketList::KDSoapSocketList(KDSoapServer* server)
    : m_server(server), m_serverObject(server->createServerObject())
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
    QObject::connect(socket, SIGNAL(disconnected()),
                     socket, SLOT(deleteLater()));
    m_sockets.insert(socket);
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
