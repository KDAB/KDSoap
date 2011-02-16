#ifndef KDSOAPTHREADPOOL_H
#define KDSOAPTHREADPOOL_H

#include <QObject>
#include <QHash>
#include "KDSoapServerGlobal.h"
class KDSoapServer;

/**
 * Pool of threads that can be used to handle SOAP requests in a SOAP server.
 * The thread pool is configured with a maximum number of threads.
 *
 * In case the server application provides different services on different ports,
 * it can decide to use the same thread pool for both services, in order
 * to always respect the maximum number of threads globally.
 */
class KDSOAPSERVER_EXPORT KDSoapThreadPool : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a thread pool with the given \p parent.
     */
    KDSoapThreadPool(QObject* parent = 0);

    /**
     * Destructs the thread pool
     */
    ~KDSoapThreadPool();

    /**
     * Sets the maximum number of threads used by the thread pool.
     * Note: The thread pool will always use at least 1 thread, even if \p maxThreadCount
     * limit is zero or negative.
     * The default maxThreadCount is QThread::idealThreadCount().
     */
    void setMaxThreadCount(int maxThreadCount);

    /**
     * Returns the maximum number of threads used by the thread pool.
     */
    int maxThreadCount() const;

private:
    friend class KDSoapServer;
    void handleIncomingConnection(int socketDescriptor, KDSoapServer* server);
    class Private;
    Private* const d;
};

#endif // KDSOAPTHREADPOOL_H
