/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapPendingCall.h"
#include "KDSoapMessageReader_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapPendingCall_p.h"
#include <QDebug>
#include <QNetworkReply>

static void debugHelper(const QByteArray &data, const QList<QNetworkReply::RawHeaderPair> &headerList)
{
    const QByteArray doDebug = qgetenv("KDSOAP_DEBUG");
    const QList<QByteArray> options = doDebug.toLower().split(',');
    const bool optEscape = options.contains("escape");
    const bool optHttp = options.contains("http") || options.contains("https");
    const bool optReformat = options.contains("reformat");
    quint8 indentation = 4;
    for (const QByteArray &opt : qAsConst(options)) {
        if (opt.startsWith("indent=")) { // krazy:exclude=strings
            indentation = opt.mid(7).toUShort();
        }
    }

    QByteArray toOutput;
    if (optHttp) {
        for (const QNetworkReply::RawHeaderPair &header : qAsConst(headerList)) {
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
        qDebug() << toOutput;
    } else {
        qDebug().noquote() << toOutput;
    }
}

// Log the HTTP and XML of a response from the server.
static void maybeDebugResponse(const QByteArray &data, QNetworkReply *reply)
{
    const QByteArray doDebug = qgetenv("KDSOAP_DEBUG");
    if (doDebug.trimmed().isEmpty() || doDebug == "0") {
        return;
    }

    debugHelper(data, reply->rawHeaderPairs());
}

// Log the HTTP and XML of a request.
// (not static, because this is used in KDSoapClientInterface)
void maybeDebugRequest(const QByteArray &data, const QNetworkRequest &request, QNetworkReply *reply)
{
    const QByteArray doDebug = qgetenv("KDSOAP_DEBUG");
    if (doDebug.trimmed().isEmpty() || doDebug == "0") {
        return;
    }

    QList<QNetworkReply::RawHeaderPair> headerList;
    if (reply) {
        QByteArray method;
        switch (reply->operation()) {
        default:
            break; // don't try to mimic the basic HTTP command
        case QNetworkAccessManager::GetOperation:
            method = "GET";
            break;
        case QNetworkAccessManager::HeadOperation:
            method = "HEAD";
            break;
        case QNetworkAccessManager::PutOperation:
            method = "PUT";
            break;
        case QNetworkAccessManager::PostOperation:
            method = "POST";
            break;
        case QNetworkAccessManager::DeleteOperation:
            method = "DELETE";
            break;
        }
        if (!method.isEmpty()) {
            QByteArray output = method + " " + reply->url().toString().toUtf8();
            headerList << QNetworkReply::RawHeaderPair {{}, std::move(output)};
        }
    }
    const auto rawHeaders = request.rawHeaderList();
    for (const QByteArray &h : rawHeaders) {
        headerList << QNetworkReply::RawHeaderPair {h, request.rawHeader(h)};
    }
    debugHelper(data, headerList);
}

KDSoapPendingCall::Private::~Private()
{
    if (reply) {
        // Ensure the connection is closed, which QNetworkReply doesn't do in its destructor. This needs abort().
        QObject::disconnect(reply.data(), &QNetworkReply::finished, nullptr, nullptr);
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
    if (!reply->isFinished()) {
        qWarning("KDSoap: Parsing reply before it finished!");
        return;
    }
    parsed = true;

    // Don't try to read from an aborted (closed) reply
    const QByteArray data = reply->isOpen() ? reply->readAll() : QByteArray();
    maybeDebugResponse(data, reply);

    if (!data.isEmpty()) {
        KDSoapMessageReader reader;
        reader.xmlToMessage(data, &replyMessage, nullptr, &replyHeaders, this->soapVersion);
    }

    if (reply->error()) {
        if (!replyMessage.isFault()) {
            replyHeaders.clear();
            if (reply->error() == QNetworkReply::OperationCanceledError
                && reply->property("kdsoap_reply_timed_out").toBool()) { // see KDSoapClientInterface.cpp
                replyMessage.createFaultMessage(QString::number(QNetworkReply::TimeoutError), QLatin1String("Operation timed out"), soapVersion);
            } else {
                replyMessage.createFaultMessage(QString::number(reply->error()), reply->errorString(), soapVersion);
            }
        }
    }
}
