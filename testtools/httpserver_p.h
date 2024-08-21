/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef HTTPSERVER_P_H
#define HTTPSERVER_P_H

#include "KDSoapGlobal.h"
#include <QBuffer>
#include <QMutex>
#include <QSemaphore>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#endif
#include <QSslCertificate>
#include <QSslKey>
#include <QStringList>
#include <QUrl>

class BlockingHttpServer;

namespace KDSoapUnitTestHelpers {
bool xmlBufferCompare(const QByteArray &source, const QByteArray &dest);
void httpGet(const QUrl &url);
bool setSslConfiguration();
const char *xmlEnvBegin11();
const char *xmlEnvBegin11WithWSAddressing();
const char *xmlEnvBegin12();
const char *xmlEnvBegin12WithWSAddressing();
const char *xmlEnvEnd();
}

class HttpServerThread : public QThread
{
    Q_OBJECT
public:
    enum Feature
    {
        Public = 0, // HTTP with no ssl and no authentication needed
        Ssl = 1, // HTTPS
        BasicAuth = 2, // Requires authentication
        Error404 = 4 // Return "404 not found"
                     // bitfield, next item is 8
    };
    Q_DECLARE_FLAGS(Features, Feature)

    HttpServerThread(const QByteArray &dataToSend, Features features)
        : m_dataToSend(dataToSend)
        , m_port(0)
        , m_features(features)
        , m_server(0)
    {
        start();
        m_ready.acquire();
    }
    ~HttpServerThread()
    {
        finish();
        wait();
    }

    void disableSsl();
    inline int serverPort() const
    {
        QMutexLocker lock(&m_mutex);
        return m_port;
    }
    QString endPoint() const
    {
        return QString::fromLatin1("%1://127.0.0.1:%2/path").arg(QString::fromLatin1((m_features & Ssl) ? "https" : "http")).arg(serverPort());
    }

    inline void finish()
    {
        KDSoapUnitTestHelpers::httpGet(QUrl(endPoint() + QLatin1String("/terminateThread")));
    }

    QByteArray receivedData() const
    {
        QMutexLocker lock(&m_mutex);
        return m_receivedData;
    }
    QByteArray receivedHeaders() const
    {
        QMutexLocker lock(&m_mutex);
        return m_receivedHeaders;
    }
    void resetReceivedBuffers()
    {
        QMutexLocker lock(&m_mutex);
        m_receivedData.clear();
        m_receivedHeaders.clear();
    }

    QByteArray header(const QByteArray &value) const
    {
        QMutexLocker lock(&m_mutex);
        return m_headers.value(value, m_headers.value(value.toLower()));
    }

    /**
     * @brief clientUseWSAddressing
     * @return
     * Returns true if the server expects clients to use WS-Addressing.
     */
    bool clientSendsActionInHttpHeader() const;

    /**
     * @brief setClientSendsActionInHttpHeader
     * @param clientSendsActionInHttpHeader
     * If this function is called with clientSendsActionInHttpHeader set to false then
     * (in the case of SOAP 1.2 requests) the Content-type HTTP header will not checked
     * for containing the proper SOAP action. In this case the client should send the SOAP action
     * embedded to the SOAP envelope's header section).
     */
    void setClientSendsActionInHttpHeader(bool clientSendsActionInHttpHeader);

    /**
     * @brief setExpectedSoapAction
     * @param expectedSoapAction
     * This function could be used to specify the expected SOAP action in the incoming request's header.
     * In the case of SOAP 1.1 requests the SOAP action should be in the SoapAction HTTP header,
     * in the case of SOAP 1.2 requests the SOAP action is sent in the Content-Type HTTP header's
     * action parameter. Please note if setClientSendsActionInHttpHeader is called with false argument
     * then this check is skipped. (In this case the SOAP action should be passed in the envelope's
     * header). To disable the SOAP action checking call this function with an empty expectedSoapAction
     * parameter. (This is the default behaviour of the HttpServerThread).
     */
    void setExpectedSoapAction(const QByteArray &expectedSoapAction);

    /**
     * @brief expectedSoapAction
     * @return the expected SOAP action against which the request's headers will be checked.
     * If the function returns an empty string then the SOAP actions of the incoming requests
     * will not be checked.
     */
    QByteArray expectedSoapAction() const;

protected:
    /* \reimp */ void run() override;

private:
    enum Method
    {
        None,
        Basic,
        Plain,
        Login,
        Ntlm,
        CramMd5,
        DigestMd5
    };
    static void parseAuthLine(const QString &str, Method *method, QString *headerVal)
    {
        *method = None;
        // The code below (from QAuthenticatorPrivate::parseHttpResponse)
        // is supposed to be run in a loop, apparently
        // (multiple WWW-Authenticate lines? multiple values in the line?)

        // qDebug() << "parseAuthLine() " << str;
        if (*method < Basic && str.startsWith(QLatin1String("Basic"), Qt::CaseInsensitive)) {
            *method = Basic;
            *headerVal = str.mid(6);
        } else if (*method < Ntlm && str.startsWith(QLatin1String("NTLM"), Qt::CaseInsensitive)) {
            *method = Ntlm;
            *headerVal = str.mid(5);
        } else if (*method < DigestMd5 && str.startsWith(QLatin1String("Digest"), Qt::CaseInsensitive)) {
            *method = DigestMd5;
            *headerVal = str.mid(7);
        }
    }

    QByteArray makeHttpResponse(const QByteArray &responseData)
    {
        QByteArray httpResponse;
        // ### missing here: error code 500 if response is a fault
        if (m_features & Error404) {
            httpResponse += "HTTP/1.1 404 Not Found\r\n";
        } else {
            httpResponse += "HTTP/1.1 200 OK\r\n";
        }
        httpResponse += "Content-Type: text/xml\r\nContent-Length: ";
        httpResponse += QByteArray::number(responseData.size());
        httpResponse += "\r\n";

        // We don't support multiple connexions so let's ask the client
        // to close the connection every time. See testCallNoReply which performs
        // multiple connexions at the same time (QNAM keeps the old connection open).
        httpResponse += "Connection: close\r\n";
        httpResponse += "\r\n";
        httpResponse += responseData;
        return httpResponse;
    }

private:
    QByteArray m_partialRequest;
    QSemaphore m_ready;
    QByteArray m_dataToSend;

    mutable QMutex m_mutex; // protects the 4 vars below
    QByteArray m_receivedData;
    QByteArray m_receivedHeaders;
    QMap<QByteArray, QByteArray> m_headers;
    int m_port;

    Features m_features;
    BlockingHttpServer *m_server;
    bool m_clientSendsActionInHttpHeader = true;
    QByteArray m_expectedSoapAction;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HttpServerThread::Features)

// KDSoap-server-side testing
// We need to do the listening and socket handling in a separate thread,
// so that the main thread can use synchronous calls. Note that this is
// really specific to unit tests and doesn't need to be done in a real
// KDSoap-based server.
template<class ServerObjectType>
class TestServerThread
{
public:
    TestServerThread()
        : m_thread(0)
        , m_pServer(0)
    {
    }
    ~TestServerThread()
    {
        if (m_thread) {
            m_thread->quit();
            m_thread->wait();
            delete m_thread;
        }
    }
    ServerObjectType *startThread()
    {
        m_pServer = new ServerObjectType;
        if (!m_pServer->listen()) {
            delete m_pServer;
            m_pServer = 0;
            return 0;
        }

        m_thread = new QThread;
        QObject::connect(m_thread, &QThread::finished, m_pServer, &ServerObjectType::deleteLater);

        m_pServer->moveToThread(m_thread);
        m_thread->start();
        return m_pServer;
    }

private:
    QThread *m_thread; // we could also use m_pServer->thread()
    ServerObjectType *m_pServer;
};

#endif /* HTTPSERVER_P_H */
