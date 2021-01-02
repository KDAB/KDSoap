/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#ifndef KDSOAPUDPCLIENT_P_H
#define KDSOAPUDPCLIENT_P_H

#include <QObject>
#include <QUdpSocket>

#include "KDSoapUdpClient.h"

class KDSoapUdpClientPrivate : public QObject
{
    Q_OBJECT
public:
    explicit KDSoapUdpClientPrivate(KDSoapUdpClient *q)
        : q_ptr(q), socketIPv4(0), socketIPv6(0)
    {}

    void receivedDatagram(const QByteArray &messageData, const QHostAddress &senderAddress, quint16 senderPort);

public Q_SLOTS:
    void readyRead();

public:
    QUdpSocket *socketIPv4;
    QUdpSocket *socketIPv6;
    KDSoap::SoapVersion soapVersion = KDSoap::SOAP1_2;

private:
    KDSoapUdpClient *const q_ptr;
    Q_DECLARE_PUBLIC(KDSoapUdpClient)
};

#endif // KDSOAPUDPCLIENT_P_H
