/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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
#include "KDSoapServerObjectInterface.h"
#include "KDSoapServerSocket_p.h"
#include <QDebug>
#include <QPointer>

class KDSoapServerObjectInterface::Private
{
public:
    Private() :
        m_serverSocket(0)
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
    // QPointer in case the client disconnects during a delayed response
    QPointer<KDSoapServerSocket> m_serverSocket;
};

KDSoapServerObjectInterface::KDSoapServerObjectInterface()
    : d(new Private)
{
}

KDSoapServerObjectInterface::~KDSoapServerObjectInterface()
{
    delete d;
}

void KDSoapServerObjectInterface::processRequest(const KDSoapMessage &request, KDSoapMessage& response, const QByteArray& soapAction)
{
    const QString method = request.name();
    qDebug() << "Slot not found:" << method << "[soapAction =" << soapAction << "]" /* << "in" << metaObject()->className()*/;
    response.setFault(true);
    response.addArgument(QString::fromLatin1("faultcode"), QString::fromLatin1("Server.MethodNotFound"));
    response.addArgument(QString::fromLatin1("faultstring"), QString::fromLatin1("%1 not found").arg(method));
}

QIODevice *KDSoapServerObjectInterface::processFileRequest(const QString &path, QByteArray &contentType)
{
    Q_UNUSED(path);
    Q_UNUSED(contentType);
    return 0;
}

void KDSoapServerObjectInterface::processRequestWithPath(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray &soapAction, const QString &path)
{
    Q_UNUSED(soapAction);
    const QString method = request.name();
    qWarning("Invalid path: \"%s\"", qPrintable(path));
    //qWarning() << "Invalid path:" << path << "[method =" << method << "; soapAction =" << soapAction << "]" /* << "in" << metaObject()->className()*/;
    response.setFault(true);
    response.addArgument(QString::fromLatin1("faultcode"), QString::fromLatin1("Client.Data"));
    response.addArgument(QString::fromLatin1("faultstring"), QString::fromLatin1("Method %1 not found in path %2").arg(method, path));
}

void KDSoapServerObjectInterface::doneProcessingRequestWithPath(const KDSoapServerObjectInterface &otherInterface)
{
    d->m_faultCode = otherInterface.d->m_faultCode;
    d->m_faultString = otherInterface.d->m_faultString;
    d->m_faultActor = otherInterface.d->m_faultActor;
    d->m_detail = otherInterface.d->m_detail;
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

void KDSoapServerObjectInterface::storeFaultAttributes(KDSoapMessage& message) const
{
    // SOAP 1.1  <faultcode>, <faultstring>, <faultfactor>, <detail>
    message.addArgument(QString::fromLatin1("faultcode"), d->m_faultCode);
    message.addArgument(QString::fromLatin1("faultstring"), d->m_faultString);
    message.addArgument(QString::fromLatin1("faultactor"), d->m_faultActor);
    if (d->m_detailValue.isNil() || d->m_detailValue.isNull())
        message.addArgument(QString::fromLatin1("detail"), d->m_detail);
    else {
        KDSoapValueList detailAsList;
        detailAsList.append( d->m_detailValue );
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

void KDSoapServerObjectInterface::setRequestHeaders(const KDSoapHeaders &headers, const QByteArray& soapAction)
{
    d->m_requestHeaders = headers;
    d->m_soapAction = soapAction;
    // Prepare for a new request to be handled
    d->m_faultCode.clear();
    d->m_responseHeaders.clear();
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
}

void KDSoapServerObjectInterface::sendDelayedResponse(const KDSoapDelayedResponseHandle& responseHandle, const KDSoapMessage &response)
{
    KDSoapServerSocket* socket = responseHandle.serverSocket();
    if (socket)
        socket->sendDelayedReply(this, response);
}

void KDSoapServerObjectInterface::writeHTTP(const QByteArray &httpReply)
{
#if !defined(NDEBUG)
    const qint64 written =
#endif
    d->m_serverSocket->write(httpReply);
    Q_ASSERT(written == httpReply.size()); // Please report a bug if you hit this.
}

void KDSoapServerObjectInterface::writeXML(const QByteArray &reply, bool isFault)
{
    d->m_serverSocket->writeXML(reply, isFault);
}

void KDSoapServerObjectInterface::setResponseNamespace(const QString& ns)
{
    d->m_responseNamespace = ns;
}

QString KDSoapServerObjectInterface::responseNamespace() const
{
    return d->m_responseNamespace;
}
