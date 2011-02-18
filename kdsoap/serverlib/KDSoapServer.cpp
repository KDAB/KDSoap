#include "KDSoapServer.h"
#include "KDSoapThreadPool.h"
#include "KDSoapSocketList_p.h"

class KDSoapServer::Private
{
public:
    Private()
        : m_threadPool(new KDSoapThreadPool),
          m_ownThreadPool(true),
          m_mainThreadSocketList(0),
          m_use(KDSoapMessage::LiteralUse)
    {
    }

    ~Private()
    {
        if (m_ownThreadPool) {
            delete m_threadPool;
        }
        delete m_mainThreadSocketList;
    }

    KDSoapThreadPool* m_threadPool;
    bool m_ownThreadPool;
    KDSoapSocketList* m_mainThreadSocketList;
    KDSoapServer::LogLevel m_logLevel;
    KDSoapMessage::Use m_use;
};

KDSoapServer::KDSoapServer(QObject* parent)
    : QTcpServer(parent),
      d(new KDSoapServer::Private)
{
    setMaxPendingConnections(1000); // TODO see if this works...
}

KDSoapServer::~KDSoapServer()
{
    delete d->m_mainThreadSocketList;
    delete d;
}

void KDSoapServer::incomingConnection(int socketDescriptor)
{
    if (d->m_threadPool) {
        d->m_threadPool->handleIncomingConnection(socketDescriptor, this);
    } else {
        if (!d->m_mainThreadSocketList)
            d->m_mainThreadSocketList = new KDSoapSocketList(this /*server*/);
        d->m_mainThreadSocketList->handleIncomingConnection(socketDescriptor);
    }
}

void KDSoapServer::setThreadPool(KDSoapThreadPool *threadPool)
{
    d->m_threadPool = threadPool;
    d->m_ownThreadPool = false;
}

KDSoapThreadPool * KDSoapServer::threadPool() const
{
    return d->m_threadPool;
}

QString KDSoapServer::endPoint() const {
    const QHostAddress address = serverAddress();
    const QString addressStr = address == QHostAddress::Any ? QString::fromLatin1("127.0.0.1") : address.toString();
    return QString::fromLatin1("%1://%2:%3/path")
            .arg(QString::fromLatin1(/*(m_features & Ssl)?"https":*/"http"))
            .arg(addressStr)
            .arg(serverPort());
}

void KDSoapServer::setLogLevel(KDSoapServer::LogLevel level)
{
    // TODO use somewhere :-)
    d->m_logLevel = level;
}

KDSoapServer::LogLevel KDSoapServer::logLevel() const
{
    return d->m_logLevel;
}

void KDSoapServer::setUse(KDSoapMessage::Use use)
{
    d->m_use = use;
}

KDSoapMessage::Use KDSoapServer::use() const
{
    return d->m_use;
}

#include "moc_KDSoapServer.cpp"
