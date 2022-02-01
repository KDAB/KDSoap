/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/
#include "KDSoapClientInterface.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapMessageWriter_p.h"
#ifndef QT_NO_SSL
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
    : m_accessManager(nullptr)
    , m_authentication()
    , m_version(KDSoap::SOAP1_1)
    , m_style(KDSoapClientInterface::RPCStyle)
    , m_ignoreSslErrors(false)
    , m_timeout(30 * 60 * 1000) // 30 minutes, as documented
{
#ifndef QT_NO_SSL
    m_sslHandler = nullptr;
#endif
}

KDSoapClientInterfacePrivate::~KDSoapClientInterfacePrivate()
{
#ifndef QT_NO_SSL
    delete m_sslHandler;
#endif
}

QNetworkAccessManager *KDSoapClientInterfacePrivate::accessManager()
{
    if (!m_accessManager) {
        m_accessManager = new QNetworkAccessManager(this);
        connect(m_accessManager, &QNetworkAccessManager::authenticationRequired, this, &KDSoapClientInterfacePrivate::_kd_slotAuthenticationRequired);
    }
    return m_accessManager;
}

QNetworkRequest KDSoapClientInterfacePrivate::prepareRequest(const QString &method, const QString &action)
{
    QNetworkRequest request(QUrl(this->m_endPoint));

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // HTTP/2 (on by default since Qt 6) creates trouble, disable it for now (https://github.com/KDAB/KDSoap/issues/246)
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
#endif

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
    // qDebug() << "soapAction=" << soapAction;

    QString soapHeader;
    if (m_version == KDSoap::SOAP1_1) {
        soapHeader += QString::fromLatin1("text/xml;charset=utf-8");
        request.setRawHeader("SoapAction", '\"' + soapAction.toUtf8() + '\"');
    } else if (m_version == KDSoap::SOAP1_2) {
        soapHeader += QString::fromLatin1("application/soap+xml;charset=utf-8");
        if (m_sendSoapActionInHttpHeader)
            soapHeader += QString::fromLatin1(";action=") + soapAction;
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

#ifndef QT_NO_SSL
    if (!m_sslConfiguration.isNull()) {
        request.setSslConfiguration(m_sslConfiguration);
    }
#endif

    return request;
}

QBuffer *KDSoapClientInterfacePrivate::prepareRequestBuffer(const QString &method, const KDSoapMessage &message, const QString &soapAction, const KDSoapHeaders &headers)
{
    KDSoapMessageWriter msgWriter;
    msgWriter.setMessageNamespace(m_messageNamespace);
    msgWriter.setVersion(m_version);
    QBuffer *buffer = new QBuffer;
    auto setBufferData = [=](const KDSoapMessage &msg) {
        buffer->setData(msgWriter.messageToXml(msg,
                                                     (m_style == KDSoapClientInterface::RPCStyle) ? method : QString(),
                                                     headers, m_persistentHeaders,
                                                     m_authentication));
    };

    if (m_sendSoapActionInWsAddressingHeader) {
        KDSoapMessage messageCopy = message;
        KDSoapMessageAddressingProperties prop = message.messageAddressingProperties();
        if (!prop.action().isEmpty())
            qWarning("Overwriting the action addressing parameter (%s) with the SOAP action (%s)",
                     prop.action().toLocal8Bit().constData(), soapAction.toLocal8Bit().constData());
        prop.setAction(soapAction);
        messageCopy.setMessageAddressingProperties(prop);
        setBufferData(messageCopy);
    } else {
        setBufferData(message);
    }
    buffer->open(QIODevice::ReadOnly);
    return buffer;
}

KDSoapPendingCall KDSoapClientInterface::asyncCall(const QString &method, const KDSoapMessage &message, const QString &soapAction,
                                                   const KDSoapHeaders &headers)
{
    QBuffer *buffer = d->prepareRequestBuffer(method, message, soapAction, headers);
    QNetworkRequest request = d->prepareRequest(method, soapAction);
    QNetworkReply *reply = d->accessManager()->post(request, buffer);
    d->setupReply(reply);
    maybeDebugRequest(buffer->data(), reply->request(), reply);
    KDSoapPendingCall call(reply, buffer);
    call.d->soapVersion = d->m_version;
    return call;
}

KDSoapMessage KDSoapClientInterface::call(const QString &method, const KDSoapMessage &message, const QString &soapAction,
                                          const KDSoapHeaders &headers)
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

void KDSoapClientInterface::callNoReply(const QString &method, const KDSoapMessage &message,
                                        const QString &soapAction, const KDSoapHeaders &headers)
{
    QBuffer *buffer = d->prepareRequestBuffer(method, message, soapAction, headers);
    QNetworkRequest request = d->prepareRequest(method, soapAction);
    QNetworkReply *reply = d->accessManager()->post(request, buffer);
    d->setupReply(reply);
    maybeDebugRequest(buffer->data(), reply->request(), reply);
    QObject::connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    QObject::connect(reply, &QNetworkReply::finished, buffer, &QBuffer::deleteLater);
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

#ifndef QT_NO_SSL
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
#ifndef QT_NO_SSL
    if (m_ignoreSslErrors) {
        QObject::connect(reply, &QNetworkReply::sslErrors, reply, QOverload<>::of(&QNetworkReply::ignoreSslErrors));
    } else {
        reply->ignoreSslErrors(m_ignoreErrorsList);
        if (m_sslHandler) {
            // create a child object of the reply, which will forward to m_sslHandler.
            // this is a workaround for the lack of the reply pointer in the signal,
            // and sender() doesn't work for sync calls (from another thread) (SOAP-79/issue29)
            new KDSoapReplySslHandler(reply, m_sslHandler);
        }
    }
#endif
    if (m_timeout >= 0) {
        TimeoutHandler *timeoutHandler = new TimeoutHandler(reply);
        connect(timeoutHandler, &TimeoutHandler::timeout, timeoutHandler, &TimeoutHandler::replyTimeout);
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

bool KDSoapClientInterface::sendSoapActionInHttpHeader() const
{
    return d->m_sendSoapActionInHttpHeader;
}

void KDSoapClientInterface::setSendSoapActionInHttpHeader(bool sendInHttpHeader)
{
    d->m_sendSoapActionInHttpHeader = sendInHttpHeader;
}

bool KDSoapClientInterface::sendSoapActionInWsAddressingHeader() const
{
    return d->m_sendSoapActionInWsAddressingHeader;
}

void KDSoapClientInterface::setSendSoapActionInWsAddressingHeader(bool sendInWsAddressingHeader)
{
    d->m_sendSoapActionInWsAddressingHeader = sendInWsAddressingHeader;
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
