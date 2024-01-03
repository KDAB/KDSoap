/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPPENDINGCALL_P_H
#define KDSOAPPENDINGCALL_P_H

#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include <QBuffer>
#include <QNetworkReply>
#include <QPointer>
#include <QSharedData>
#include <QXmlStreamReader>

class KDSoapValue;

void maybeDebugRequest(const QByteArray &data, const QNetworkRequest &request, QNetworkReply *reply);

class KDSoapPendingCall::Private : public QSharedData
{
public:
    Private(QNetworkReply *r, QBuffer *b)
        : reply(r)
        , buffer(b)
        , soapVersion(KDSoap::SOAP1_1)
        , parsed(false)
    {
    }
    ~Private();

    void parseReply();
    KDSoapValue parseReplyElement(QXmlStreamReader &reader);

    // Can be deleted under us if the KDSoapClientInterface (and its QNetworkAccessManager)
    // are deleted before the KDSoapPendingCall.
    QPointer<QNetworkReply> reply;
    QBuffer *buffer;
    KDSoapMessage replyMessage;
    KDSoapHeaders replyHeaders;
    KDSoap::SoapVersion soapVersion;
    bool parsed;
};

#endif // KDSOAPPENDINGCALL_P_H
