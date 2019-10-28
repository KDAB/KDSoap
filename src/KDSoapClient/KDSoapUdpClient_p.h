/*
 * Copyright (C) 2019  Casper Meijn <casper@meijn.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KDSOAPUDPCLIENT_P_H
#define KDSOAPUDPCLIENT_P_H

#include <QObject>
#include <QUdpSocket>

#include "KDSoapUdpClient.h"

class KDSoapUdpClientPrivate : public QObject
{
    Q_OBJECT
public:
    KDSoapUdpClientPrivate(KDSoapUdpClient *q) 
        : q_ptr(q)
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
