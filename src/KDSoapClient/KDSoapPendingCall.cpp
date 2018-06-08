/****************************************************************************
** Copyright (C) 2010-2018 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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
#include "KDSoapPendingCall.h"
#include "KDSoapPendingCall_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapMessageReader_p.h"
#include <QNetworkReply>
#include <QDebug>

KDSoapPendingCall::Private::~Private()
{
    if (reply) {
        // Ensure the connection is closed, which QNetworkReply doesn't do in its destructor. This needs abort().
        QObject::disconnect(reply.data(), SIGNAL(finished()), 0, 0);
        reply->abort();
    }
    delete reply.data();
    delete buffer;
}

KDSoapPendingCall::KDSoapPendingCall(QNetworkReply *reply, QBuffer *buffer)
    : d(new Private(reply, buffer))
{
}

KDSoapPendingCall::KDSoapPendingCall(const KDSoapPendingCall &other)
    : d(other.d)
{
}

KDSoapPendingCall::~KDSoapPendingCall()
{
}

KDSoapPendingCall &KDSoapPendingCall::operator=(const KDSoapPendingCall &other)
{
    d = other.d;
    return *this;
}

bool KDSoapPendingCall::isFinished() const
{
#if QT_VERSION >= 0x040600
    return d->reply.data()->isFinished();
#else
    return false;
#endif
}

KDSoapMessage KDSoapPendingCall::returnMessage() const
{
    d->parseReply();
    return d->replyMessage;
}

KDSoapHeaders KDSoapPendingCall::returnHeaders() const
{
    d->parseReply();
    return d->replyHeaders;
}

QVariant KDSoapPendingCall::returnValue() const
{
    d->parseReply();
    if (!d->replyMessage.childValues().isEmpty()) {
        return d->replyMessage.childValues().first().value();
    }
    return QVariant();
}

void KDSoapPendingCall::Private::parseReply()
{
    if (parsed) {
        return;
    }
    const bool doDebug = qgetenv("KDSOAP_DEBUG").toInt();
    QNetworkReply *reply = this->reply.data();
#if QT_VERSION >= 0x040600
    if (!reply->isFinished()) {
        qWarning("KDSoap: Parsing reply before it finished!");
        return;
    }
#endif
    parsed = true;
    const QByteArray data = reply->readAll();
    if (doDebug) {
        qDebug() << data;
    }

    if (!data.isEmpty()) {
        KDSoapMessageReader reader;
        reader.xmlToMessage(data, &replyMessage, 0, &replyHeaders, this->soapVersion);
    }

    if (reply->error()) {
        if (!replyMessage.isFault()) {
            replyHeaders.clear();
            replyMessage = KDSoapMessage();
            replyMessage.setFault(true);
            if (this->soapVersion == KDSoapClientInterface::SOAP1_2) {
                replyMessage.setNamespaceUri(QString::fromLatin1("http://www.w3.org/2003/05/soap-envelope"));
                KDSoapValueList codeValueList;
                codeValueList.addArgument(QString::fromLatin1("Value"), QString::number(reply->error()));
                replyMessage.addArgument(QString::fromLatin1("Code"), codeValueList);
                KDSoapValueList reasonValueList;
                reasonValueList.addArgument(QString::fromLatin1("Text"), reply->errorString());
                replyMessage.addArgument(QString::fromLatin1("Reason"), reasonValueList);
            } else {
                replyMessage.addArgument(QString::fromLatin1("faultcode"), QString::number(reply->error()));
                replyMessage.addArgument(QString::fromLatin1("faultstring"), reply->errorString());
            }
        }
    }
}
