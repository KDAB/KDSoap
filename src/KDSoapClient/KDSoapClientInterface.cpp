/****************************************************************************
** Copyright (C) 2010-2019 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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
#include "KDSoapClientInterface.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapMessageWriter_p.h"
#ifndef QT_NO_OPENSSL
#include "KDSoapSslHandler.h"
#include "KDSoapReplySslHandler_p.h"
#endif
#include "KDSoapPendingCall_p.h"
#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDebug>
#include <QBuffer>
#include <QNetworkProxy>
#include <QTimer>

KDSoapClientInterface::KDSoapClientInterface(const QString &endPoint, const QString &messageNamespace)
    : d(new KDSoapClientInterfacePrivate)
{
    d->m_endPoint = endPoint;
    d->m_messageNamespace = messageNamespace;
    d->m_version = KDSoap::SOAP1_1;
}

KDSoapClientInterface::~KDSoapClientInterface()
{
    d->m_thread.stop();
    d->m_thread.wait();
    delete d;
}

void KDSoapClientInterface::setSoapVersion(KDSoapClientInterface::SoapVersion version)
{
    d->m_version = static_cast<KDSoap::SoapVersion>(version);
}

KDSoapClientInterface::SoapVersion KDSoapClientInterface::soapVersion() const
{
    return static_cast<KDSoapClientInterface::SoapVersion>(d->m_version);
}

KDSoapClientInterfacePrivate::KDSoapClientInterfacePrivate()
    : m_accessManager(nullptr),
      m_authentication(),
      m_version(KDSoap::SOAP1_1),
      m_style(KDSoapClientInterface::RPCStyle),
      m_ignoreSslErrors(false),
      m_timeout(30 * 60 * 1000) // 30 minutes, as documented
{
#ifndef QT_NO_OPENSSL
    m_sslHandler = nullptr;
#endif
}

KDSoapClientInterfacePrivate::~KDSoapClientInterfacePrivate()
{
#ifndef QT_NO_OPENSSL
    delete m_sslHandler;
#endif
}

QNetworkAccessManager *KDSoapClientInterfacePrivate::accessManager()
{
    if (!m_accessManager) {
        m_accessManager = new QNetworkAccessManager(this);
        connect(m_accessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                this, SLOT(_kd_slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    }
    return m_accessManager;
}

QNetworkRequest KDSoapClientInterfacePrivate::prepareRequest(const QString &method, const QString &action)
{
    QNetworkRequest request(QUrl(this->m_endPoint));

    QString soapAction = action;

    if (soapAction.isNull()) {
        // The automatic generation of SoapAction done in this block is a mistake going back to KDSoap 1.0.
        // The spec says "there is no default value for SoapAction" (https://www.w3.org/TR/wsdl#_soap:operation)
        // but we keep this for compatibility, when nothing was passed as argument (see the webcalls unittest)
        soapAction = this->m_messageNamespace;
        if (!soapAction.endsWith(QLatin1Char('/'))) {
            soapAction += QLatin1Char('/');
        }
        soapAction += method;
    }
    //qDebug() << "soapAction=" << soapAction;

    QString soapHeader;
    if (m_version == KDSoap::SOAP1_1) {
        soapHeader += QString::fromLatin1("text/xml;charset=utf-8");
        request.setRawHeader("SoapAction", '\"' + soapAction.toUtf8() + '\"');
    } else if (m_version == KDSoap::SOAP1_2) {
        soapHeader += QString::fromLatin1("application/soap+xml;charset=utf-8;action=") + soapAction;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, soapHeader.toUtf8());

    // FIXME need to find out which version of Qt this is no longer necessary
    // without that the server might respond with gzip compressed data and
    // Qt 4.6.2 fails to decode that properly
    //
    // happens with retrieval calls in against SugarCRM 5.5.1 running on Apache 2.2.15
    // when the response seems to reach a certain size threshold
    request.setRawHeader("Accept-Encoding", "compress");

    for (QMap<QByteArray, QByteArray>::const_iterator it = m_httpHeaders.constBegin(); it != m_httpHeaders.constEnd(); ++it) {
        request.setRawHeader(it.key(), it.value());
    }

#ifndef QT_NO_OPENSSL
    if (!m_sslConfiguration.isNull()) {
        request.setSslConfiguration(m_sslConfiguration);
    }
#endif

    return request;
}

QBuffer *KDSoapClientInterfacePrivate::prepareRequestBuffer(const QString &method, const KDSoapMessage &message, const KDSoapHeaders &headers)
{
    KDSoapMessageWriter msgWriter;
    msgWriter.setMessageNamespace(m_messageNamespace);
    msgWriter.setVersion(m_version);
    const QByteArray data = msgWriter.messageToXml(message, (m_style == KDSoapClientInterface::RPCStyle) ? method : QString(), headers, m_persistentHeaders, m_authentication);
    QBuffer *buffer = new QBuffer;
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);
    return buffer;
}

KDSoapPendingCall KDSoapClientInterface::asyncCall(const QString &method, const KDSoapMessage &message, const QString &soapAction, const KDSoapHeaders &headers)
{
    QBuffer *buffer = d->prepareRequestBuffer(method, message, headers);
    QNetworkRequest request = d->prepareRequest(method, soapAction);
    QNetworkReply *reply = d->accessManager()->post(request, buffer);
    d->setupReply(reply);
    maybeDebugRequest(buffer->data(), reply->request(), reply);
    KDSoapPendingCall call(reply, buffer);
    call.d->soapVersion = d->m_version;
    return call;
}

KDSoapMessage KDSoapClientInterface::call(const QString &method, const KDSoapMessage &message, const QString &soapAction, const KDSoapHeaders &headers)
{
    d->accessManager()->cookieJar(); // create it in the right thread, the secondary thread will use it
    // Problem is: I don't want a nested event loop here. Too dangerous for GUI programs.
    // I wanted a socket->waitFor... but we don't have access to the actual socket in QNetworkAccess.
    // So the only option that remains is a thread and acquiring a semaphore...
    KDSoapThreadTaskData *task = new KDSoapThreadTaskData(this, method, message, soapAction, headers);
    task->m_authentication = d->m_authentication;
    d->m_thread.enqueue(task);
    if (!d->m_thread.isRunning()) {
        d->m_thread.start();
    }
    task->waitForCompletion();
    KDSoapMessage ret = task->response();
    d->m_lastResponseHeaders = task->responseHeaders();
    delete task;
    return ret;
}

void KDSoapClientInterface::callNoReply(const QString &method, const KDSoapMessage &message, const QString &soapAction, const KDSoapHeaders &headers)
{
    QBuffer *buffer = d->prepareRequestBuffer(method, message, headers);
    QNetworkRequest request = d->prepareRequest(method, soapAction);
    QNetworkReply *reply = d->accessManager()->post(request, buffer);
    d->setupReply(reply);
    maybeDebugRequest(buffer->data(), reply->request(), reply);
    QObject::connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
    QObject::connect(reply, SIGNAL(finished()), buffer, SLOT(deleteLater()));
}

void KDSoapClientInterfacePrivate::_kd_slotAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    m_authentication.handleAuthenticationRequired(reply, authenticator);
}

void KDSoapClientInterface::setAuthentication(const KDSoapAuthentication &authentication)
{
    d->m_authentication = authentication;
}

QString KDSoapClientInterface::endPoint() const
{
    return d->m_endPoint;
}

void KDSoapClientInterface::setEndPoint(const QString &endPoint)
{
    d->m_endPoint = endPoint;
}

void KDSoapClientInterface::setHeader(const QString &name, const KDSoapMessage &header)
{
    d->m_persistentHeaders[name] = header;
    d->m_persistentHeaders[name].setQualified(true);
}

void KDSoapClientInterface::ignoreSslErrors()
{
    d->m_ignoreSslErrors = true;
}

#ifndef QT_NO_OPENSSL
void KDSoapClientInterface::ignoreSslErrors(const QList<QSslError> &errors)
{
    d->m_ignoreErrorsList = errors;
}
#endif

// Workaround for lack of connect-to-lambdas in Qt4
// The pure Qt5 code could read like
/*
    QTimer *timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, reply, [reply]() { contents_of_the_slot });
*/
class TimeoutHandler : public QTimer // this way a single QObject is needed
{
    Q_OBJECT
public:
    TimeoutHandler(QNetworkReply *reply)
        : QTimer(reply)
    {
        setSingleShot(true);
    }
public Q_SLOTS:
    void replyTimeout()
    {
        QNetworkReply *reply = qobject_cast<QNetworkReply *>(parent());
        Q_ASSERT(reply);

        // contents_of_the_slot:
        reply->setProperty("kdsoap_reply_timed_out", true); // see KDSoapPendingCall.cpp
        reply->abort();
    }
};

void KDSoapClientInterfacePrivate::setupReply(QNetworkReply *reply)
{
    if (m_ignoreSslErrors) {
        QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
    } else {
#ifndef QT_NO_OPENSSL
        reply->ignoreSslErrors(m_ignoreErrorsList);
        if (m_sslHandler) {
            // create a child object of the reply, which will forward to m_sslHandler.
            // this is a workaround for the lack of the reply pointer in the signal,
            // and sender() doesn't work for sync calls (from another thread) (SOAP-79/issue29)
            new KDSoapReplySslHandler(reply, m_sslHandler);
        }
#endif
    }
    if (m_timeout >= 0) {
        TimeoutHandler *timeoutHandler = new TimeoutHandler(reply);
        connect(timeoutHandler, SIGNAL(timeout()), timeoutHandler, SLOT(replyTimeout()));
        timeoutHandler->start(m_timeout);
    }
}

KDSoapHeaders KDSoapClientInterface::lastResponseHeaders() const
{
    return d->m_lastResponseHeaders;
}

void KDSoapClientInterface::setStyle(KDSoapClientInterface::Style style)
{
    d->m_style = style;
}

KDSoapClientInterface::Style KDSoapClientInterface::style() const
{
    return d->m_style;
}

QNetworkCookieJar *KDSoapClientInterface::cookieJar() const
{
    return d->accessManager()->cookieJar();
}

void KDSoapClientInterface::setCookieJar(QNetworkCookieJar *jar)
{
    QObject *oldParent = jar->parent();
    d->accessManager()->setCookieJar(jar);
    jar->setParent(oldParent); // see comment in QNAM::setCookieJar...
}

void KDSoapClientInterface::setRawHTTPHeaders(const QMap<QByteArray, QByteArray> &headers)
{
    d->m_httpHeaders = headers;
}

QNetworkProxy KDSoapClientInterface::proxy() const
{
    return d->accessManager()->proxy();
}

void KDSoapClientInterface::setProxy(const QNetworkProxy &proxy)
{
    d->accessManager()->setProxy(proxy);
}

int KDSoapClientInterface::timeout() const
{
    return d->m_timeout;
}

void KDSoapClientInterface::setTimeout(int msecs)
{
    d->m_timeout = msecs;
}

#ifndef QT_NO_OPENSSL
QSslConfiguration KDSoapClientInterface::sslConfiguration() const
{
    return d->m_sslConfiguration;
}

void KDSoapClientInterface::setSslConfiguration(const QSslConfiguration &config)
{
    d->m_sslConfiguration = config;
}

KDSoapSslHandler *KDSoapClientInterface::sslHandler() const
{
    if (!d->m_sslHandler) {
        d->m_sslHandler = new KDSoapSslHandler;
    }
    return d->m_sslHandler;
}
#endif

#include "moc_KDSoapClientInterface_p.cpp"
#include "KDSoapClientInterface.moc"
