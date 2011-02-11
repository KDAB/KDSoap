#include "KDSoapSocketPool_p.h"
#include "KDSoapThreadPool.h"

KDSoapThreadPool::KDSoapThreadPool(QObject* parent)
    : QObject(parent),
      m_mainThreadSocketPool(new KDSoapSocketPool(this))
{
}

void KDSoapThreadPool::handleIncomingConnection(int socketDescriptor)
{
    // TODO KDSoapServerThread : public QThread
    m_mainThreadSocketPool->handleIncomingConnection(socketDescriptor);
}

#include "moc_KDSoapThreadPool.cpp"
