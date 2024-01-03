/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapUdpClient.h"
#include "KDSoapUdpClient_p.h"

#include "KDSoapMessage.h"
#include "KDSoapMessageReader_p.h"
#include "KDSoapMessageWriter_p.h"
#include <QNetworkInterface>

static bool isMulticastAddress(const QHostAddress &address)
{
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        return address.isInSubnet(QHostAddress(QLatin1String("224.0.0.0")), 4);
    } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        return address.isInSubnet(QHostAddress(QLatin1String("ff00::")), 8);
    }
    return false;
}

KDSoapUdpClient::KDSoapUdpClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new KDSoapUdpClientPrivate(this))
{
    Q_D(KDSoapUdpClient);
    d->socket = new QUdpSocket(this);
    connect(d->socket, &QIODevice::readyRead, d, &KDSoapUdpClientPrivate::readyRead);
}

KDSoapUdpClient::~KDSoapUdpClient()
{
    delete d_ptr;
}

bool KDSoapUdpClient::bind(quint16 port, QAbstractSocket::BindMode mode)
{
    Q_D(KDSoapUdpClient);
    const bool rc = d->socket->bind(QHostAddress::Any, port, mode);
    if (!rc) {
        qWarning() << "KDSoapUdpClient: failed to bind on port" << port << "mode" << mode << ":" << d->socket->errorString();
    }
    return rc;
}

void KDSoapUdpClient::setSoapVersion(KDSoap::SoapVersion version)
{
    Q_D(KDSoapUdpClient);
    d->soapVersion = version;
}

bool KDSoapUdpClient::sendMessage(const KDSoapMessage &message, const KDSoapHeaders &headers, const QHostAddress &address, quint16 port)
{
    Q_D(KDSoapUdpClient);
    KDSoapMessageWriter msgWriter;
    msgWriter.setVersion(d->soapVersion);
    const QByteArray data = msgWriter.messageToXml(message, QString(), headers, QMap<QString, KDSoapMessage>());

    if (isMulticastAddress(address)) {
        bool anySuccess = false;
        const auto &allInterfaces = QNetworkInterface::allInterfaces();
        for (const auto &iface : allInterfaces) {
            if (iface.flags().testFlag(QNetworkInterface::IsUp) && iface.flags().testFlag(QNetworkInterface::CanMulticast)) {
                // qDebug() << "Sending multicast to" << iface.name() << address << ":" << data;
                d->socket->setMulticastInterface(iface);
                const qint64 writtenSize = d->socket->writeDatagram(data, address, port);
                anySuccess = anySuccess || (writtenSize == data.size());
            }
        }
        return anySuccess;
    } else {
        // qDebug() << "Sending to" << address << ":" << data;
        const qint64 writtenSize = d->socket->writeDatagram(data, address, port);
        return writtenSize == data.size();
    }
}

void KDSoapUdpClientPrivate::readyRead()
{
    QUdpSocket *socket = qobject_cast<QUdpSocket *>(sender());
    while (socket->hasPendingDatagrams()) {
        qint64 size = socket->pendingDatagramSize();

        QByteArray buffer;
        buffer.resize(size);
        QHostAddress senderAddress;
        quint16 senderPort;
        socket->readDatagram(buffer.data(), buffer.size(), &senderAddress, &senderPort);

        receivedDatagram(buffer, senderAddress, senderPort);
    }
}

void KDSoapUdpClientPrivate::receivedDatagram(const QByteArray &messageData, const QHostAddress &senderAddress, quint16 senderPort)
{
    Q_Q(KDSoapUdpClient);
    // qDebug() << "Received datagram from:" << senderAddress << "data:" << QString::fromUtf8(messageData);

    KDSoapMessage replyMessage;
    KDSoapHeaders replyHeaders;

    KDSoapMessageReader reader;
    reader.xmlToMessage(messageData, &replyMessage, 0, &replyHeaders, soapVersion);

    emit q->receivedMessage(replyMessage, replyHeaders, senderAddress, senderPort);
}
