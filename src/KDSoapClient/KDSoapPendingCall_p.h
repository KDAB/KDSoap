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
