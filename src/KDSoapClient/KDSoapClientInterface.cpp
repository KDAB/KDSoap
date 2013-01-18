/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
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
#include "KDSoapClientInterface.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapMessageWriter_p.h"
#ifndef QT_NO_OPENSSL
#include "KDSoapSslHandler.h"
#endif
#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDebug>
#include <QBuffer>
#include <QNetworkProxy>

KDSoapClientInterface::KDSoapClientInterface(const QString& endPoint, const QString& messageNamespace)
    : d(new KDSoapClientInterfacePrivate)
{
    d->m_endPoint = endPoint;
    d->m_messageNamespace = messageNamespace;
    d->m_version = SOAP1_1;
}

KDSoapClientInterface::~KDSoapClientInterface()
{
    d->m_thread.stop();
    d->m_thread.wait();
    delete d;
}

void KDSoapClientInterface::setSoapVersion(KDSoapClientInterface::SoapVersion version)
{
    d->m_version = version;
}

KDSoapClientInterface::SoapVersion KDSoapClientInterface::soapVersion()
{
  return d->m_version;
}


KDSoapClientInterfacePrivate::KDSoapClientInterfacePrivate()
    : m_authentication(),
      m_style(KDSoapClientInterface::RPCStyle),
      m_ignoreSslErrors(false)
{
#ifndef QT_NO_OPENSSL
    m_sslHandler = 0;
#endif
    connect(&m_accessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(_kd_slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    m_accessManager.cookieJar(); // create it in the right thread...
}

KDSoapClientInterfacePrivate::~KDSoapClientInterfacePrivate()
{
#ifndef QT_NO_OPENSSL
    delete m_sslHandler;
#endif
}

QNetworkRequest KDSoapClientInterfacePrivate::prepareRequest(const QString &method, const QString& action)
{
    QNetworkRequest request(QUrl(this->m_endPoint));

    // The soap action seems to be namespace + method in most cases, but not always
    // (e.g. urn:GoogleSearchAction for google).
    QString soapAction = action;
    if (soapAction.isEmpty()) {
        // Does the namespace always end with a '/'? - nope, it doesn't.
        soapAction = this->m_messageNamespace;
        if (!soapAction.endsWith(QLatin1Char('/')))
            soapAction += QLatin1Char('/');
        soapAction += method;
    }
    //qDebug() << "soapAction=" << soapAction;

    QString soapHeader;
    if (m_version == KDSoapClientInterface::SOAP1_1) {
        soapHeader += QString::fromLatin1("text/xml;charset=utf-8");
        request.setRawHeader("SoapAction", '\"' + soapAction.toUtf8() + '\"');
    } else if (m_version == KDSoapClientInterface::SOAP1_2) {
        soapHeader += QString::fromLatin1("application/soap+xml;charset=utf-8;action=") + soapAction;
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, soapHeader.toUtf8());

    // FIXME need to find out which version of Qt this is no longer necessary
    // without that the server might respond with gzip compressed data and
    // Qt 4.6.2 fails to decode that properly
    //
    // happens with retrieval calls in against SugarCRM 5.5.1 running on Apache 2.2.15
    // when the response seems to reach a certain size threshold
    request.setRawHeader( "Accept-Encoding", "compress" );

#ifndef QT_NO_OPENSSL
    if (!m_sslConfiguration.isNull())
        request.setSslConfiguration(m_sslConfiguration);
#endif

    return request;
}

QBuffer* KDSoapClientInterfacePrivate::prepareRequestBuffer(const QString& method, const KDSoapMessage& message, const KDSoapHeaders& headers)
{
    KDSoapMessageWriter msgWriter;
    msgWriter.setMessageNamespace(m_messageNamespace);
    msgWriter.setVersion(m_version);
    const QByteArray data = msgWriter.messageToXml(message, (m_style == KDSoapClientInterface::RPCStyle) ? method : QString(), headers, m_persistentHeaders);
    QBuffer* buffer = new QBuffer;
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);
    return buffer;
}

KDSoapPendingCall KDSoapClientInterface::asyncCall(const QString &method, const KDSoapMessage &message, const QString& soapAction, const KDSoapHeaders& headers)
{
    QBuffer* buffer = d->prepareRequestBuffer(method, message, headers);
    QNetworkRequest request = d->prepareRequest(method, soapAction);
    //qDebug() << "post()";
    QNetworkReply* reply = d->m_accessManager.post(request, buffer);
    d->setupReply(reply);
    return KDSoapPendingCall(reply, buffer);
}

KDSoapMessage KDSoapClientInterface::call(const QString& method, const KDSoapMessage &message, const QString& soapAction, const KDSoapHeaders& headers)
{
    // Problem is: I don't want a nested event loop here. Too dangerous for GUI programs.
    // I wanted a socket->waitFor... but we don't have access to the actual socket in QNetworkAccess.
    // So the only option that remains is a thread and acquiring a semaphore...
    KDSoapThreadTaskData* task = new KDSoapThreadTaskData(this, method, message, soapAction, headers);
    task->m_authentication = d->m_authentication;
    d->m_thread.enqueue(task);
    if (!d->m_thread.isRunning())
        d->m_thread.start();
    task->waitForCompletion();
    KDSoapMessage ret = task->response();
    d->m_lastResponseHeaders = task->responseHeaders();
    delete task;
    return ret;
}

void KDSoapClientInterface::callNoReply(const QString &method, const KDSoapMessage &message, const QString &soapAction, const KDSoapHeaders& headers)
{
    QBuffer* buffer = d->prepareRequestBuffer(method, message, headers);
    QNetworkRequest request = d->prepareRequest(method, soapAction);
    QNetworkReply* reply = d->m_accessManager.post(request, buffer);
    d->setupReply(reply);
    QObject::connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
}

void KDSoapClientInterfacePrivate::_kd_slotAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
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

void KDSoapClientInterface::setHeader(const QString& name, const KDSoapMessage &header)
{
    d->m_persistentHeaders[name] = header;
    d->m_persistentHeaders[name].setQualified(true);
}

void KDSoapClientInterface::ignoreSslErrors()
{
    d->m_ignoreSslErrors = true;
}

void KDSoapClientInterfacePrivate::setupReply(QNetworkReply *reply)
{
    if (m_ignoreSslErrors) {
        QObject::connect(reply, SIGNAL(sslErrors(const QList<QSslError>&)), reply, SLOT(ignoreSslErrors()));
    } else {
#ifndef QT_NO_OPENSSL
        if (m_sslHandler) {
            QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), m_sslHandler, SLOT(slotSslErrors(QList<QSslError>)));
        }
#endif
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

QNetworkCookieJar * KDSoapClientInterface::cookieJar() const
{
    return d->m_accessManager.cookieJar();
}

void KDSoapClientInterface::setCookieJar(QNetworkCookieJar *jar)
{
    QObject* oldParent = jar->parent();
    d->m_accessManager.setCookieJar(jar);
    jar->setParent(oldParent); // see comment in QNAM::setCookieJar...
}

QNetworkProxy KDSoapClientInterface::proxy() const
{
    return d->m_accessManager.proxy();
}

void KDSoapClientInterface::setProxy(const QNetworkProxy &proxy)
{
    d->m_accessManager.setProxy(proxy);
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

KDSoapSslHandler* KDSoapClientInterface::sslHandler() const
{
    if (!d->m_sslHandler)
        d->m_sslHandler = new KDSoapSslHandler;
    return d->m_sslHandler;
}
#endif

#include "moc_KDSoapClientInterface_p.cpp"
