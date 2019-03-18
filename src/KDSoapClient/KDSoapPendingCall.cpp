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
#include "KDSoapPendingCall.h"
#include "KDSoapPendingCall_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapMessageReader_p.h"
#include <QNetworkReply>
#include <QDebug>

static void debugHelper(const QByteArray &data, const QList<QNetworkReply::RawHeaderPair> &headerList) {
    const QByteArray doDebug = qgetenv("KDSOAP_DEBUG");
    const QList<QByteArray> options = doDebug.toLower().split(',');
    const bool optEscape = options.contains("escape");
    const bool optHttp = options.contains("http") || options.contains("https");
    const bool optReformat = options.contains("reformat");
    quint8 indentation = 4;
    Q_FOREACH( const QByteArray &opt, options ) {
        if (opt.startsWith("indent=")) { //krazy:exclude=strings
            indentation = opt.mid(7).toUShort();
        }
    }

    QByteArray toOutput;
    if (optHttp) {
        Q_FOREACH( const QNetworkReply::RawHeaderPair &header, headerList ) {
            if (!header.first.isEmpty()) {
                toOutput += header.first + ": ";
            }
            toOutput += header.second + "\n";
        }
        toOutput += "\n";
    }

    if (optReformat) {
        QByteArray reformatted;
        QXmlStreamReader reader(data);
        QXmlStreamWriter writer(&reformatted);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(indentation);

        while (!reader.atEnd()) {
            reader.readNext();
            if (!reader.hasError() && !reader.isWhitespace()) {
                writer.writeCurrentToken(reader);
            }
        }

        toOutput += reader.hasError() ? data : reformatted;
    } else {
        toOutput += data;
    }

    if (optEscape) {
        // no support for escaping with Qt4
        qDebug() << toOutput;
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
        qDebug().noquote() << toOutput;
#else
        qDebug() << toOutput;
#endif
    }
}

// Log the HTTP and XML of a response from the server.
static void maybeDebugResponse(const QByteArray &data, QNetworkReply *reply) {
    const QByteArray doDebug = qgetenv("KDSOAP_DEBUG");
    if (doDebug.trimmed().isEmpty() || doDebug == "0") {
        return;
    }

    debugHelper(data, reply->rawHeaderPairs());
}

// Log the HTTP and XML of a request.
// (not static, because this is used in KDSoapClientInterface)
void maybeDebugRequest(const QByteArray &data, const QNetworkRequest &request, QNetworkReply *reply) {
    const QByteArray doDebug = qgetenv("KDSOAP_DEBUG");
    if (doDebug.trimmed().isEmpty() || doDebug == "0") {
        return;
    }

    QList<QNetworkReply::RawHeaderPair> headerList;
    if (reply) {
        QByteArray method;
        switch (reply->operation()) {
            default: break; // don't try to mimic the basic HTTP command
            case QNetworkAccessManager::GetOperation: method = "GET"; break;
            case QNetworkAccessManager::HeadOperation: method = "HEAD"; break;
            case QNetworkAccessManager::PutOperation: method = "PUT"; break;
            case QNetworkAccessManager::PostOperation: method = "POST"; break;
            case QNetworkAccessManager::DeleteOperation: method = "DELETE"; break;
        }
        if (!method.isEmpty()) {
            headerList << qMakePair<QByteArray,QByteArray>("", method + " " + qPrintable(reply->url().toString()));
        }
    }
    Q_FOREACH( const QByteArray &h, request.rawHeaderList() ) {
        headerList << qMakePair<QByteArray,QByteArray>(h, request.rawHeader(h));
    }
    debugHelper(data, headerList);
}


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
    return d->reply.data()->isFinished();
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
    QNetworkReply *reply = this->reply.data();
#if QT_VERSION >= 0x040600
    if (!reply->isFinished()) {
        qWarning("KDSoap: Parsing reply before it finished!");
        return;
    }
#endif
    parsed = true;

    // Don't try to read from an aborted (closed) reply
    const QByteArray data = reply->isOpen() ? reply->readAll() : QByteArray();
    maybeDebugResponse(data, reply);

    if (!data.isEmpty()) {
        KDSoapMessageReader reader;
        reader.xmlToMessage(data, &replyMessage, 0, &replyHeaders, this->soapVersion);
    }

    if (reply->error()) {
        if (!replyMessage.isFault()) {
            replyHeaders.clear();
            if (reply->error() == QNetworkReply::OperationCanceledError && reply->property("kdsoap_reply_timed_out").toBool()) // see KDSoapClientInterface.cpp
                replyMessage.createFaultMessage(QString::number(QNetworkReply::TimeoutError), QLatin1String("Operation timed out"), soapVersion);
            else
                replyMessage.createFaultMessage(QString::number(reply->error()), reply->errorString(), soapVersion);
        }
    }
}
