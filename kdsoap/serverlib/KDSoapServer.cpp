#include "KDSoapServer.h"
#include "KDSoapThreadPool.h"

class KDSoapServer::Private
{
public:
    Private()
        : m_threadPool(new KDSoapThreadPool),
          m_ownThreadPool(true)
    {

    }

    ~Private()
    {
        if (m_ownThreadPool) {
            delete m_threadPool;
        }
    }

    KDSoapThreadPool* m_threadPool;
    bool m_ownThreadPool;
};

KDSoapServer::KDSoapServer(QObject* parent)
    : QTcpServer(parent), d(new KDSoapServer::Private)
{
    setMaxPendingConnections(1000); // TODO see if this works...
}

KDSoapServer::~KDSoapServer()
{
    delete d;
}

void KDSoapServer::incomingConnection(int socketDescriptor)
{
    d->m_threadPool->handleIncomingConnection(socketDescriptor);
}

void KDSoapServer::setThreadPool(KDSoapThreadPool *threadPool)
{
    d->m_threadPool = threadPool;
    d->m_ownThreadPool = false;
}

QString KDSoapServer::endPoint() const {
    const QHostAddress address = serverAddress();
    const QString addressStr = address == QHostAddress::Any ? QString::fromLatin1("127.0.0.1") : address.toString();
    return QString::fromLatin1("%1://%2:%3/path")
            .arg(QString::fromLatin1(/*(m_features & Ssl)?"https":*/"http"))
            .arg(addressStr)
            .arg(serverPort());
}

#include "moc_KDSoapServer.cpp"
