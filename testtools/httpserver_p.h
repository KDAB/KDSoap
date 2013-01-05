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

#ifndef HTTPSERVER_P_H
#define HTTPSERVER_P_H

#include "KDSoapGlobal.h"
#include <QBuffer>
#include <QThread>
#include <QSemaphore>
#include <QTcpServer>
#include <QTcpSocket>
#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#endif
#include <QUrl>
#include <QStringList>
#include <QSslCertificate>
#include <QSslKey>

class BlockingHttpServer;

namespace KDSoapUnitTestHelpers
{
    bool xmlBufferCompare(const QByteArray& source, const QByteArray& dest);
    void httpGet(const QUrl& url);
    bool setSslConfiguration();
}

class HttpServerThread : public QThread
{
    Q_OBJECT
public:
    enum Feature {
        Public = 0,    // HTTP with no ssl and no authentication needed
        Ssl = 1,       // HTTPS
        BasicAuth = 2,  // Requires authentication
        Error404 = 4   // Return "404 not found"
        // bitfield, next item is 8
    };
    Q_DECLARE_FLAGS(Features, Feature)

    HttpServerThread(const QByteArray& dataToSend, Features features)
        : m_dataToSend(dataToSend), m_features(features)
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
    inline int serverPort() const { return m_port; }
    QString endPoint() const {
        return QString::fromLatin1("%1://127.0.0.1:%2/path")
                           .arg(QString::fromLatin1((m_features & Ssl)?"https":"http"))
                           .arg(serverPort());
    }

    inline void finish() {
        KDSoapUnitTestHelpers::httpGet(endPoint() + QLatin1String("/terminateThread"));
    }

    QByteArray receivedData() const { return m_receivedData; }
    QByteArray receivedHeaders() const { return m_receivedHeaders; }
    void resetReceivedBuffers() {
        m_receivedData.clear();
        m_receivedHeaders.clear();
    }

    QByteArray header(const QByteArray& value) const {
        return m_headers.value(value);
    }

protected:
    /* \reimp */ void run();

private:

    enum Method { None, Basic, Plain, Login, Ntlm, CramMd5, DigestMd5 };
    static void parseAuthLine(const QString& str, Method* method, QString* headerVal)
    {
        *method = None;
        // The code below (from QAuthenticatorPrivate::parseHttpResponse)
        // is supposed to be run in a loop, apparently
        // (multiple WWW-Authenticate lines? multiple values in the line?)

        //qDebug() << "parseAuthLine() " << str;
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

    QByteArray makeHttpResponse(const QByteArray& responseData)
    {
        QByteArray httpResponse;
        // ### missing here: error code 500 if response is a fault
        if (m_features & Error404)
            httpResponse += "HTTP/1.1 404 Not Found\r\n";
        else
            httpResponse += "HTTP/1.1 200 OK\r\n";
        httpResponse += "Content-Type: text/xml\r\nContent-Length: ";
        httpResponse += QByteArray::number(responseData.size());
        httpResponse += "\r\n";

        // We don't support multiple connexions so let's ask the client
        // to close the connection every time. See testCallNoReply which performs
        // multiple connexions at the same time (QNAM keeps the old connexion open).
        httpResponse += "Connection: close\r\n";
        httpResponse += "\r\n";
        httpResponse += responseData;
        return httpResponse;
    }

private:
    QByteArray m_partialRequest;
    QSemaphore m_ready;
    QByteArray m_dataToSend;
    QByteArray m_receivedData;
    QByteArray m_receivedHeaders;
    QMap<QByteArray, QByteArray> m_headers;
    int m_port;
    Features m_features;
    BlockingHttpServer* m_server;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HttpServerThread::Features)

// KDSoap-server-side testing
// We need to do the listening and socket handling in a separate thread,
// so that the main thread can use synchronous calls. Note that this is
// really specific to unit tests and doesn't need to be done in a real
// KDSoap-based server.
template <class ServerObjectType>
class TestServerThread
{
public:
    TestServerThread() : m_thread(0) {}
    ~TestServerThread() {
        if (m_thread) {
            m_thread->quit();
            m_thread->wait();
            delete m_thread;
        }
    }
    ServerObjectType* startThread() {
        m_pServer = new ServerObjectType;
        if (!m_pServer->listen()) {
            delete m_pServer;
            m_pServer = 0;
            return 0;
        }

        m_thread = new QThread;
        QObject::connect(m_thread, SIGNAL(finished()), m_pServer, SLOT(deleteLater()));

        m_pServer->moveToThread(m_thread);
        m_thread->start();
        return m_pServer;
    }

private:
    QThread* m_thread; // we could also use m_pServer->thread()
    ServerObjectType* m_pServer;
};

#endif /* HTTPSERVER_P_H */

