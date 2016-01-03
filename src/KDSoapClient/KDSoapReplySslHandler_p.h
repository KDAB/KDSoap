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

#ifndef KDSOAPREPLYSSLHANDLER_P_H
#define KDSOAPREPLYSSLHANDLER_P_H

#include <QObject>
QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

#ifndef QT_NO_OPENSSL
#include <QSslError>
class KDSoapSslHandler;

class KDSoapReplySslHandler : public QObject
{
    Q_OBJECT
public:
    explicit KDSoapReplySslHandler(QNetworkReply *reply, KDSoapSslHandler *handler);

private Q_SLOTS:
    void slotReplySslErrors(const QList<QSslError> &errors);

private:
    KDSoapSslHandler *m_handler;
};

#endif
#endif // KDSOAPREPLYSSLHANDLER_P_H
