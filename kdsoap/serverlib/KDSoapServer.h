#ifndef KDSOAPSERVER_H
#define KDSOAPSERVER_H

#include "KDSoapServerGlobal.h"
#include <KDSoapMessage.h>
#include <QTcpServer>

class KDSoapThreadPool;

/**
 * HTTP soap server.
 *
 * Every instance of KDSoapServer represents one service, listening on one port.
 * Call the listen() method from QTcpServer in order to start listening on a port.
 *
 * KDSoapServer is a base class for your server, you must inherit from it
 * and reimplement the method createServerObject().
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
    explicit KDSoapServer(QObject* parent = 0);

    /**
     * Destructor.
     * Deletes the server object factory as well.
     */
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
     * Returns the thread pool for this server, or 0 if no thread pool was set.
     */
    KDSoapThreadPool* threadPool() const;

    /**
     * Returns the HTTP URL which can be used to access this server.
     * For instance "http://127.0.0.1:8000".
     *
     * If the server is listening for connections yet, returns an empty string.
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

    /**
     * Define the way the message should be serialized: with or without type information.
     * This value usually comes from the <binding> element in the WSDL service description.
     * The default value is KDSoapMessage::LiteralUse.
     * \see KDSoapMessage::Use
     */
    void setUse(KDSoapMessage::Use use);
    /**
     * Returns the value passed to setUse().
     */
    KDSoapMessage::Use use() const;

    enum LogLevel { LogNothing, LogFaults, LogEveryCall };
    /**
     * Sets the level of logging to be used by this SOAP server:
     * <ul>
     *  <li>LogNothing: no logging (the default).</li>
     *  <li>LogFaults: log all faults.</li>
     *  <li>LogEveryCall: log every call, successful or not.</li>
     * </ul>
     *
     * Warning: enabling logging reduces performance severely. Not only
     * because of the time spent logging, but also because the threads can
     * only write one at a time to the file, to avoid mixed output.
     */
    void setLogLevel(LogLevel level);
    /**
     * Returns the level of logging set by setLogLevel.
     */
    LogLevel logLevel() const;

    /**
     * Sets the name of the file where logging should go.
     * The server always appends to this file, you should delete it
     * or rename it first if you don't want an ever-growing log file.
     */
    void setLogFileName(const QString& fileName);

    /**
     * Returns the name of the log file given to setLogFileName().
     */
    QString logFileName() const;

    /**
     * Force flushing the log file to disk.
     */
    void flushLogFile();

    /**
     * Sets the number of expected sockets in this process.
     * This is necessary in order to increase system limits when a large number of clients
     * is expected.
     *
     * The special value -1 means "as many as possible in this non-root process".
     * Only processes running as root can set the absolute maximum to an arbitrary value.
     */
    static bool setExpectedSocketCount(int sockets);

    /**
     * Returns the number of connected sockets.
     * This information can change at any time, and is therefore only useful
     * for statistical purposes.
     */
    int numConnectedSockets() const;

    /**
     * Set the full path to the .wsdl file (including the filename), so that it can be
     * downloaded by clients using endPoint() + the filename of the wsdl file.
     * (For instance http://myserver.example.com/myservice.wsdl)
     */
    void setWsdlFile(const QString& file);

    /**
     * \returns the path given to setWsdlFile
     */
    QString wsdlFile() const;

public Q_SLOTS:
    /**
     * Temporarily suspend (do not listen to incoming connections, and close all
     * connected sockets after servicing current requests).
     */
    void suspend();

    /**
     * Resume activity after suspend
     */
    void resume();

protected:
    /*! \reimp \internal */ void incomingConnection(int socketDescriptor);

private:
    friend class KDSoapServerSocket;
    void log(const QByteArray& text);
    class Private;
    Private* const d;
};

#endif
