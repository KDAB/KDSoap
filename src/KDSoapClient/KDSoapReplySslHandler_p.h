/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#ifndef KDSOAPREPLYSSLHANDLER_P_H
#define KDSOAPREPLYSSLHANDLER_P_H

#include <QObject>
#include <QSslError>
QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

class KDSoapSslHandler;

#ifndef QT_NO_SSL

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
