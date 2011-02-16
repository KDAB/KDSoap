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
    /**
     * Constructs a Soap Server.
     *
     * By default it will not use threads to handle requests, see setThreadPool for that.
     */
    KDSoapServer(QObject* parent = 0);

    /**
     * Destructor.
     * Deletes the server object factory as well.
     */
    ~KDSoapServer();

    // TODO setOptions use=encoded/literal, style=rpc/document

    /**
     * Sets the thread pool for this server.
     * This is useful if you want to share a thread pool between multiple server instances,
     * in order to ensure an overall maximum of threads, across multiple services.
     * An existing thread pool will be removed, but not deleted.
     * KDSoapServer does not take ownership of the thread pool.
     */
    void setThreadPool(KDSoapThreadPool* threadPool);

    /**
     * Returns the thread pool for this server, or 0 if no thread pool was set.
     */
    KDSoapThreadPool* threadPool() const;

    /**
     * Returns the HTTP URL which can be used to access this server.
     * For instance "http://127.0.0.1:8000".
     */
    QString endPoint() const;

    /**
     * Reimplement this method to create an application-specific server object
     * to handle incoming requests.
     * Important: the created object must derive from KDSoapServerObjectInterface
     * and must use Q_INTERFACES(KDSoapServerObjectInterface) under the Q_OBJECT macro.
     *
     * When using a thread pool, this method will be called from different threads.
     * The server takes ownership of the created object.
     */
    virtual QObject* createServerObject() = 0;

protected:
    /*! \reimp */ void incomingConnection(int socketDescriptor);

private:
    class Private;
    Private* const d;
};

#endif
