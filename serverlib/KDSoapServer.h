#ifndef KDSOAPSERVER_H
#define KDSOAPSERVER_H

#include "KDSoapServerGlobal.h"
#include <QTcpServer>

class KDSoapThreadPool;

/**
 * HTTP soap server - see QTcpServer for API
 * [TODO expand docu]
 */
class KDSOAPSERVER_EXPORT KDSoapServer : public QTcpServer
{
    Q_OBJECT
public:
    KDSoapServer(QObject* parent = 0);
    ~KDSoapServer();

    /**
     * Sets the thread pool for this server.
     * This is useful if you want to share a thread pool between multiple server instances,
     * in order to ensure an overall maximum of threads, across multiple services.
     * An existing thread pool will be removed, but not deleted.
     * KDSoapServer does not take ownership of the thread pool.
     */
    void setThreadPool(KDSoapThreadPool* threadPool);

    /**
     * Returns the HTTP URL which can be used to access this server.
     * For instance "http://127.0.0.1:8000".
     */
    QString endPoint() const;

protected:
    /*! \reimp */ void incomingConnection(int socketDescriptor);

private:
    class Private;
    Private* const d;
};

#endif
