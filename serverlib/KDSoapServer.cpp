#include "KDSoapServer.h"
#include "KDSoapThreadPool.h"
#include "KDSoapSocketList_p.h"
#include <QMutex>
#include <QFile>

class KDSoapServer::Private
{
public:
    Private()
        : m_threadPool(new KDSoapThreadPool),
          m_ownThreadPool(true),
          m_mainThreadSocketList(0),
          m_use(KDSoapMessage::LiteralUse),
          m_logLevel(KDSoapServer::LogNothing)
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
    KDSoapMessage::Use m_use;

    QMutex m_logMutex;
    KDSoapServer::LogLevel m_logLevel;
    QString m_logFileName;
    QFile m_logFile;
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

void KDSoapServer::setUse(KDSoapMessage::Use use)
{
    d->m_use = use;
}

KDSoapMessage::Use KDSoapServer::use() const
{
    return d->m_use;
}

void KDSoapServer::setLogLevel(KDSoapServer::LogLevel level)
{
    QMutexLocker lock(&d->m_logMutex);
    d->m_logLevel = level;
}

KDSoapServer::LogLevel KDSoapServer::logLevel() const
{
    QMutexLocker lock(&d->m_logMutex);
    return d->m_logLevel;
}

void KDSoapServer::setLogFileName(const QString &fileName)
{
    QMutexLocker lock(&d->m_logMutex);
    d->m_logFileName = fileName;
}

QString KDSoapServer::logFileName() const
{
    QMutexLocker lock(&d->m_logMutex);
    return d->m_logFileName;
}

void KDSoapServer::log(const QByteArray &text)
{
    QMutexLocker lock(&d->m_logMutex);
    if (!d->m_logFile.isOpen() && !d->m_logFileName.isEmpty()) {
        d->m_logFile.setFileName(d->m_logFileName);
        if (!d->m_logFile.open(QIODevice::Append)) {
            qCritical("Could not open log file for writing: %s", qPrintable(d->m_logFileName));
            d->m_logFileName.clear(); // don't retry every time log() is called
            return;
        }
    }
    d->m_logFile.write(text);
}

#include "moc_KDSoapServer.cpp"
