/****************************************************************************
** Copyright (C) 2010-2012 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#ifndef KDSOAPSERVER_H
#define KDSOAPSERVER_H

#include "KDSoapServerGlobal.h"
#include <KDSoapClient/KDSoapMessage.h>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QSslConfiguration>

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

    enum Feature {
        Public = 0,       ///< HTTP with no ssl and no authentication needed (default)
        Ssl = 1,          ///< HTTPS
        AuthRequired = 2  ///< Requires authentication
        // bitfield, next item is 4
    };
    Q_DECLARE_FLAGS(Features, Feature)

    /**
     * Set all the features of the server that should be enabled.
     * For instance, the use of SSL, or the use of authentication.
     */
    void setFeatures(Features features);

    /**
     * Returns the features of the server that were enabled.
     */
    Features features() const;

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
     * Sets the path that the server expects in client requests.
     * By default the path is '/', but this can be changed here.
     *
     * The path is returned in endPoint(), and is checked when handling incoming requests.
     */
    void setPath(const QString& path);

    /**
     * Returns the path set by setPath()
     */
    QString path() const;


    /**
     * Returns the HTTP URL which can be used to access this server.
     * For instance "http://127.0.0.1:8000/".
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
     * Close the log file. This can be used to then rename it, in order to
     * implement log file rotation.
     */
    void closeLogFile();

    /**
     * Sets a maximum number of concurrent connections to this server.
     * When this number is reached, connections are rejected, and the signal
     * clientConnectionRejected is emitted for each rejected connection.
     *
     * The special value -1 means unlimited.
     */
    void setMaxConnections(int sockets);

    /**
     * Returns the maximum of concurrent connections as set by setMaxConnections.
     *
     * The special value -1 means unlimited.
     */
    int maxConnections() const;

    /**
     * Sets the number of expected sockets (connections) in this process.
     * This is necessary in order to increase system limits when a large number of clients
     * is expected.
     *
     * The special value -1 means "as many as possible in this non-root process".
     * Only processes running as root can set the absolute maximum to an arbitrary value.
     */
    static bool setExpectedSocketCount(int sockets);

    /**
     * Returns the number of connected sockets at this precise moment.
     * This information can change at any time, and is therefore only useful
     * for statistical purposes.
     *
     * It will always be less than maxConnections(), if maxConnections was set.
     */
    int numConnectedSockets() const;

    /**
     * Returns the number of sockets that have connected to the server since the
     * last call to resetTotalConnectionCount().
     * \since 1.2
     */
    int totalConnectionCount() const;

    /**
     * Resets totalConnectionCount to 0.
     * \since 1.2
     */
    void resetTotalConnectionCount();

    /**
     * Sets the .wsdl file that users can download from the soap server.
     * \param file relative or absolute path to the .wsdl file (including the filename), on disk
     * \param pathInUrl that clients can use in order to download the file:
     *                  for instance "/files/myservice.wsdl" for "http://myserver.example.com/files/myservice.wsdl" as final URL.
     */
    void setWsdlFile(const QString& file, const QString& pathInUrl);

    /**
     * \returns the path to the wsdl file on disk, as given to setWsdlFile
     */
    QString wsdlFile() const;

    /**
     * \returns the path given to setWsdlFile
     */
    QString wsdlPathInUrl() const;

#ifndef QT_NO_SSL
    /**
     * \returns the ssl configuration for this server
     */
    QSslConfiguration sslConfiguration() const;

    /**
     * Sets the ssl configuration to use for new server connections
     * \param config ssl configuration to use for new connections
     */
    void setSslConfiguration(const QSslConfiguration &config);
#endif

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

Q_SIGNALS:
    /**
     * Emitted when the maximum number of connections has been reached,
     * and a client connection was just rejected.
     */
    void connectionRejected();

protected:
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    /*! \reimp \internal */ void incomingConnection(qintptr socketDescriptor);
#else
    /*! \reimp \internal */ void incomingConnection(int socketDescriptor);
#endif

private:
    friend class KDSoapServerSocket;
    void log(const QByteArray& text);
    class Private;
    Private* const d;
};

#endif
