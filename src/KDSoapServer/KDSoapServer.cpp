/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#include "KDSoapServer.h"
#include "KDSoapThreadPool.h"
#include "KDSoapSocketList_p.h"
#include <QMutex>
#include <QFile>
#ifdef Q_OS_UNIX
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <limits.h>
#endif

class KDSoapServer::Private
{
public:
    Private()
        : m_threadPool(0),
          m_mainThreadSocketList(0),
          m_use(KDSoapMessage::LiteralUse),
          m_logLevel(KDSoapServer::LogNothing),
          m_path(QString::fromLatin1("/")),
          m_maxConnections(-1),
          m_portBeforeSuspend(0)
    {
    }

    ~Private()
    {
        delete m_mainThreadSocketList;
    }

    KDSoapThreadPool* m_threadPool;
    KDSoapSocketList* m_mainThreadSocketList;
    KDSoapMessage::Use m_use;
    KDSoapServer::Features m_features;

    QMutex m_logMutex;
    KDSoapServer::LogLevel m_logLevel;
    QString m_logFileName;
    QFile m_logFile;

    QMutex m_serverDataMutex;
    QString m_wsdlFile;
    QString m_wsdlPathInUrl;
    QString m_path;
    int m_maxConnections;

    QHostAddress m_addressBeforeSuspend;
    quint16 m_portBeforeSuspend;

#ifndef QT_NO_OPENSSL
    QSslConfiguration m_sslConfiguration;
#endif
};

KDSoapServer::KDSoapServer(QObject* parent)
    : QTcpServer(parent),
      d(new KDSoapServer::Private)
{
    // Probably not very useful since we handle them immediately, but cannot hurt.
    setMaxPendingConnections(1000);
}

KDSoapServer::~KDSoapServer()
{
    delete d;
}

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
void KDSoapServer::incomingConnection(qintptr socketDescriptor)
#else
void KDSoapServer::incomingConnection(int socketDescriptor)
#endif
{
    const int max = maxConnections();
    const int numSockets = numConnectedSockets();
    if (max > -1 && numSockets >= max) {
        emit connectionRejected();
        log(QByteArray("ERROR Too many connections (") + QByteArray::number(numSockets) + "), incoming connection rejected\n");
    } else if (d->m_threadPool) {
        //qDebug() << "incomingConnection: using thread pool";
        d->m_threadPool->handleIncomingConnection(socketDescriptor, this);
    } else {
        //qDebug() << "incomingConnection: using main-thread socketlist";
        if (!d->m_mainThreadSocketList)
            d->m_mainThreadSocketList = new KDSoapSocketList(this /*server*/);
        d->m_mainThreadSocketList->handleIncomingConnection(socketDescriptor);
    }
}

int KDSoapServer::numConnectedSockets() const
{
    if (d->m_threadPool) {
        return d->m_threadPool->numConnectedSockets(this);
    } else if (d->m_mainThreadSocketList) {
        return d->m_mainThreadSocketList->socketCount();
    } else {
        return 0;
    }
}

int KDSoapServer::totalConnectionCount() const
{
    if (d->m_threadPool) {
        return d->m_threadPool->totalConnectionCount(this);
    } else if (d->m_mainThreadSocketList) {
        return d->m_mainThreadSocketList->totalConnectionCount();
    } else {
        return 0;
    }
}

void KDSoapServer::resetTotalConnectionCount()
{
    if (d->m_threadPool) {
        return d->m_threadPool->resetTotalConnectionCount(this);
    } else if (d->m_mainThreadSocketList) {
        return d->m_mainThreadSocketList->resetTotalConnectionCount();
    }
}

void KDSoapServer::setThreadPool(KDSoapThreadPool *threadPool)
{
    d->m_threadPool = threadPool;
}

KDSoapThreadPool * KDSoapServer::threadPool() const
{
    return d->m_threadPool;
}

QString KDSoapServer::endPoint() const {
    const QHostAddress address = serverAddress();
    if (address == QHostAddress::Null)
        return QString();
    const QString addressStr = address == QHostAddress::Any ? QString::fromLatin1("127.0.0.1") : address.toString();
    return QString::fromLatin1("%1://%2:%3%4")
            .arg(QString::fromLatin1((d->m_features & Ssl)?"https":"http"))
            .arg(addressStr)
            .arg(serverPort())
            .arg(d->m_path);
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
    if (d->m_logLevel == KDSoapServer::LogNothing)
        return;

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

void KDSoapServer::flushLogFile()
{
    if (d->m_logFile.isOpen())
        d->m_logFile.flush();
}

void KDSoapServer::closeLogFile()
{
    if (d->m_logFile.isOpen())
        d->m_logFile.close();
}

bool KDSoapServer::setExpectedSocketCount(int sockets)
{
    // I hit a system limit when trying to connect more than 1024 sockets in the same process.
    // strace said: socket(PF_INET, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_IP) = -1 EMFILE (Too many open files)
    // Solution: ulimit -n 4096
    // Or in C code, below.

#ifdef Q_OS_UNIX
    struct rlimit lim;
    if (getrlimit(RLIMIT_NOFILE, &lim) != 0) {
        qDebug() << "error calling getrlimit:" << strerror(errno);
        return false;
    }
    bool changingHardLimit = false;
    if (sockets > -1) {
        qDebug() << "Current limit" << lim.rlim_cur << lim.rlim_max;
        sockets += 20; // we need some file descriptors too
        if (rlim_t(sockets) <= lim.rlim_cur)
            return true; // nothing to do

        if (rlim_t(sockets) > lim.rlim_max) {
            // Seems we need to run as root then
            lim.rlim_max = sockets;
            qDebug() << "Setting rlim_max to" << sockets;
            changingHardLimit = true;
        }
    }
#ifdef OPEN_MAX
    // Mac OSX: setrlimit() no longer accepts "rlim_cur = RLIM_INFINITY" for RLIM_NOFILE.  Use "rlim_cur = min(OPEN_MAX, rlim_max)".
    lim.rlim_cur = qMin(rlim_t(OPEN_MAX), lim.rlim_max);
#else
    // Linux: does not define OPEN_MAX anymore, since it's "configurable at runtime".
    lim.rlim_cur = lim.rlim_max;
#endif
    if (setrlimit(RLIMIT_NOFILE, &lim) == 0) {
        qDebug() << "limit set to" << lim.rlim_cur;
    } else {
        if (changingHardLimit) {
            qDebug() << "WARNING: hard limit is not high enough";
        }
        qDebug() << "error calling setrlimit(" << lim.rlim_cur << "," << lim.rlim_max << ") :" << strerror(errno);
        return false;
    }
#else
    Q_UNUSED(sockets);
#endif
    return true;
}

void KDSoapServer::suspend()
{
    d->m_portBeforeSuspend = serverPort();
    d->m_addressBeforeSuspend = serverAddress();
    close();

    // Disconnect connected sockets, otherwise they could still make calls
    if (d->m_threadPool) {
        d->m_threadPool->disconnectSockets(this);
    } else if (d->m_mainThreadSocketList) {
        d->m_mainThreadSocketList->disconnectAll();
    }
}

void KDSoapServer::resume()
{
    if (d->m_portBeforeSuspend == 0) {
        qWarning("KDSoapServer: resume() called without calling suspend() first");
    } else {
        if (!listen(d->m_addressBeforeSuspend, d->m_portBeforeSuspend)) {
            qWarning("KDSoapServer: failed to listen on %s port %d", qPrintable(d->m_addressBeforeSuspend.toString()), d->m_portBeforeSuspend);
        }
        d->m_portBeforeSuspend = 0;
    }
}

void KDSoapServer::setWsdlFile(const QString &file, const QString& pathInUrl)
{
    QMutexLocker lock(&d->m_serverDataMutex);
    d->m_wsdlFile = file;
    d->m_wsdlPathInUrl = pathInUrl;
}

QString KDSoapServer::wsdlFile() const
{
    QMutexLocker lock(&d->m_serverDataMutex);
    return d->m_wsdlFile;
}

QString KDSoapServer::wsdlPathInUrl() const
{
    QMutexLocker lock(&d->m_serverDataMutex);
    return d->m_wsdlPathInUrl;
}

void KDSoapServer::setPath(const QString &path)
{
    QMutexLocker lock(&d->m_serverDataMutex);
    d->m_path = path;
}

QString KDSoapServer::path() const
{
    QMutexLocker lock(&d->m_serverDataMutex);
    return d->m_path;
}

void KDSoapServer::setMaxConnections(int sockets)
{
    QMutexLocker lock(&d->m_serverDataMutex);
    d->m_maxConnections = sockets;
}

int KDSoapServer::maxConnections() const
{
    QMutexLocker lock(&d->m_serverDataMutex);
    return d->m_maxConnections;
}

void KDSoapServer::setFeatures(Features features)
{
    d->m_features = features;
}

KDSoapServer::Features KDSoapServer::features() const
{
    return d->m_features;
}

#ifndef QT_NO_OPENSSL
QSslConfiguration KDSoapServer::sslConfiguration() const
{
    return d->m_sslConfiguration;
}

void KDSoapServer::setSslConfiguration(const QSslConfiguration &config)
{
    d->m_sslConfiguration = config;
}
#endif

#include "moc_KDSoapServer.cpp"
