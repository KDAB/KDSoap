/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
#ifndef KDSOAPPENDINGCALL_P_H
#define KDSOAPPENDINGCALL_P_H

#include <QSharedData>
#include <QBuffer>
#include <QXmlStreamReader>
#include "KDSoapMessage.h"
#include <QPointer>
#include "KDSoapClientInterface.h"
#include <QNetworkReply>

class KDSoapValue;

void maybeDebugRequest(const QByteArray &data, const QNetworkRequest &request, QNetworkReply *reply);

class KDSoapPendingCall::Private : public QSharedData
{
public:
    Private(QNetworkReply *r, QBuffer *b)
        : reply(r), buffer(b), soapVersion(KDSoap::SOAP1_1), parsed(false)
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
