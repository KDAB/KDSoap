#ifndef KDSOAPTHREADPOOL_H
#define KDSOAPTHREADPOOL_H

#include <QObject>
#include <QHash>
#include "KDSoapServerGlobal.h"
class KDSoapServerObjectFactory;
class KDSoapServer;

/**
 *
 */
class KDSOAPSERVER_EXPORT KDSoapThreadPool : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs an item delegate with the given \p parent.
     */
    KDSoapThreadPool(QObject* parent = 0);

    ~KDSoapThreadPool();

    /**
     * Sets the maximum number of threads used by the thread pool.
     * Note: The thread pool will always use at least 1 thread, even if \p maxThreadCount limit is zero or negative.
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
