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

#include "KDSoapReplySslHandler_p.h"
#include "KDSoapSslHandler.h"
#include <QNetworkReply>

#ifndef QT_NO_OPENSSL

KDSoapReplySslHandler::KDSoapReplySslHandler(QNetworkReply *reply, KDSoapSslHandler *handler) :
    QObject(reply), m_handler(handler)
{
    Q_ASSERT(reply);
    Q_ASSERT(handler);
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotReplySslErrors(QList<QSslError>)));
}

void KDSoapReplySslHandler::slotReplySslErrors(const QList<QSslError> &errors)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(parent());
    Q_ASSERT(reply);
    m_handler->handleSslErrors(reply, errors);
}

#endif
