/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapServerObjectInterface.h"
#include "KDSoapClient/KDSoapValue.h"
#include "KDSoapServerSocket_p.h"
#include <QDebug>
#include <QPointer>

class KDSoapServerObjectInterface::Private
{
public:
    Private()
        : m_serverSocket(nullptr)
    {
    }

    KDSoapHeaders m_requestHeaders;
    KDSoapHeaders m_responseHeaders;
    QString m_faultCode;
    QString m_faultString;
    QString m_faultActor;
    QString m_detail;
    KDSoapValue m_detailValue;
    QString m_responseNamespace;
    QByteArray m_soapAction;
    KDSoap::SoapVersion m_requestVersion = KDSoap::SoapVersion::SOAP1_1;
    // QPointer in case the client disconnects during a delayed response
    QPointer<KDSoapServerSocket> m_serverSocket;
    QMap<QByteArray, QByteArray> m_httpHeaders;
};

KDSoapServerObjectInterface::HttpResponseHeaderItem::HttpResponseHeaderItem(const QByteArray &name, const QByteArray &value)
    : m_value(value)
    , m_name(name)
{
}

KDSoapServerObjectInterface::HttpResponseHeaderItem::HttpResponseHeaderItem()
{
}

KDSoapServerObjectInterface::KDSoapServerObjectInterface()
    : d(new Private)
{
}

KDSoapServerObjectInterface::~KDSoapServerObjectInterface()
{
    delete d;
}

void KDSoapServerObjectInterface::processRequest(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray &soapAction)
{
    const QString method = request.name();
    qDebug() << "Slot not found:" << method << "[soapAction =" << soapAction << "]" /* << "in" << metaObject()->className()*/;
    response.createFaultMessage(QString::fromLatin1("Server.MethodNotFound"), QString::fromLatin1("%1 not found").arg(method), d->m_requestVersion);
}

QIODevice *KDSoapServerObjectInterface::processFileRequest(const QString &path, QByteArray &contentType)
{
    Q_UNUSED(path);
    Q_UNUSED(contentType);
    return nullptr;
}

void KDSoapServerObjectInterface::processRequestWithPath(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray &soapAction,
                                                         const QString &path)
{
    Q_UNUSED(soapAction);
    const QString method = request.name();
    qWarning("Invalid path: \"%s\"", qPrintable(path));
    // qWarning() << "Invalid path:" << path << "[method =" << method << "; soapAction =" << soapAction << "]" /* << "in" <<
    // metaObject()->className();
    response.createFaultMessage(QString::fromLatin1("Client.Data"), QString::fromLatin1("Method %1 not found in path %2").arg(method, path),
                                d->m_requestVersion);
}

KDSoapServerObjectInterface::HttpResponseHeaderItems KDSoapServerObjectInterface::additionalHttpResponseHeaderItems() const
{
    return HttpResponseHeaderItems();
}

void KDSoapServerObjectInterface::doneProcessingRequestWithPath(const KDSoapServerObjectInterface &otherInterface)
{
    d->m_faultCode = otherInterface.d->m_faultCode;
    d->m_faultString = otherInterface.d->m_faultString;
    d->m_faultActor = otherInterface.d->m_faultActor;
    d->m_detail = otherInterface.d->m_detail;
    d->m_detailValue = otherInterface.d->m_detailValue;
    d->m_responseHeaders = otherInterface.d->m_responseHeaders;
    d->m_responseNamespace = otherInterface.d->m_responseNamespace;
}

void KDSoapServerObjectInterface::setFault(const QString &faultCode, const QString &faultString, const QString &faultActor, const QString &detail)
{
    Q_ASSERT(!faultCode.isEmpty());
    d->m_faultCode = faultCode;
    d->m_faultString = faultString;
    d->m_faultActor = faultActor;
    d->m_detail = detail;
}

void KDSoapServerObjectInterface::setFault(const QString &faultCode, const QString &faultString, const QString &faultActor, const KDSoapValue &detail)
{
    Q_ASSERT(!faultCode.isEmpty());
    d->m_faultCode = faultCode;
    d->m_faultString = faultString;
    d->m_faultActor = faultActor;
    d->m_detailValue = detail;
}

void KDSoapServerObjectInterface::storeFaultAttributes(KDSoapMessage &message) const
{
    // SOAP 1.1  <faultcode>, <faultstring>, <faultfactor>, <detail>
    message.addArgument(QString::fromLatin1("faultcode"), d->m_faultCode);
    message.addArgument(QString::fromLatin1("faultstring"), d->m_faultString);
    message.addArgument(QString::fromLatin1("faultactor"), d->m_faultActor);
    if (d->m_detailValue.isNil() || d->m_detailValue.isNull()) {
        message.addArgument(QString::fromLatin1("detail"), d->m_detail);
    } else {
        KDSoapValueList detailAsList;
        detailAsList.append(d->m_detailValue);
        message.addArgument(QString::fromLatin1("detail"), detailAsList);
    }
    // TODO  : Answer SOAP 1.2  <Code> , <Reason> , <Node> , <Role> , <Detail>
}

bool KDSoapServerObjectInterface::hasFault() const
{
    return !d->m_faultCode.isEmpty();
}

QAbstractSocket *KDSoapServerObjectInterface::serverSocket() const
{
    return d->m_serverSocket;
}

KDSoapHeaders KDSoapServerObjectInterface::requestHeaders() const
{
    return d->m_requestHeaders;
}

void KDSoapServerObjectInterface::setRequestHeaders(const KDSoapHeaders &headers, const QByteArray &soapAction)
{
    d->m_requestHeaders = headers;
    d->m_soapAction = soapAction;
    // Prepare for a new request to be handled
    d->m_faultCode.clear();
    d->m_responseHeaders.clear();
}

void KDSoapServerObjectInterface::setRequestVersion(KDSoap::SoapVersion requestVersion)
{
    d->m_requestVersion = requestVersion;
}

void KDSoapServerObjectInterface::setResponseHeaders(const KDSoapHeaders &headers)
{
    d->m_responseHeaders = headers;
}

KDSoapHeaders KDSoapServerObjectInterface::responseHeaders() const
{
    return d->m_responseHeaders;
}

QByteArray KDSoapServerObjectInterface::soapAction() const
{
    return d->m_soapAction;
}

KDSoapDelayedResponseHandle KDSoapServerObjectInterface::prepareDelayedResponse()
{
    return KDSoapDelayedResponseHandle(d->m_serverSocket);
}

void KDSoapServerObjectInterface::setServerSocket(KDSoapServerSocket *serverSocket)
{
    d->m_serverSocket = serverSocket;
    d->m_httpHeaders = d->m_serverSocket->m_httpHeaders;
}

void KDSoapServerObjectInterface::sendDelayedResponse(const KDSoapDelayedResponseHandle &responseHandle, const KDSoapMessage &response)
{
    KDSoapServerSocket *socket = responseHandle.serverSocket();
    if (socket) {
        socket->sendDelayedReply(this, response);
    }
}

void KDSoapServerObjectInterface::writeHTTP(const QByteArray &httpReply)
{
    const qint64 written = d->m_serverSocket->write(httpReply);
    Q_ASSERT(written == httpReply.size()); // Please report a bug if you hit this.
    Q_UNUSED(written);
}

void KDSoapServerObjectInterface::writeXML(const QByteArray &reply, bool isFault)
{
    d->m_serverSocket->writeXML(this, reply, isFault);
}

void KDSoapServerObjectInterface::copyFrom(KDSoapServerObjectInterface *other)
{
    d->m_requestHeaders = other->d->m_requestHeaders;
    d->m_soapAction = other->d->m_soapAction;
    d->m_serverSocket = other->d->m_serverSocket;
    d->m_httpHeaders = other->d->m_httpHeaders;
    d->m_requestVersion = other->d->m_requestVersion;
}

KDSoap::SoapVersion KDSoapServerObjectInterface::requestVersion() const
{
    return d->m_requestVersion;
}

void KDSoapServerObjectInterface::setResponseNamespace(const QString &ns)
{
    d->m_responseNamespace = ns;
}

QString KDSoapServerObjectInterface::responseNamespace() const
{
    return d->m_responseNamespace;
}
