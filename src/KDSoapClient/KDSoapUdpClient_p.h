/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2019-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
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
        : q_ptr(q)
    {
    }

    void receivedDatagram(const QByteArray &messageData, const QHostAddress &senderAddress, quint16 senderPort);

public Q_SLOTS:
    void readyRead();

public:
    QUdpSocket *socket = nullptr;
    KDSoap::SoapVersion soapVersion = KDSoap::SOAP1_2;

private:
    KDSoapUdpClient *const q_ptr;
    Q_DECLARE_PUBLIC(KDSoapUdpClient)
};

#endif // KDSOAPUDPCLIENT_P_H
