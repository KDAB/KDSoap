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

#include "KDSoapUdpClient.h"
#include "KDSoapUdpClient_p.h"

#include "KDSoapMessage.h"
#include "KDSoapMessageReader_p.h"
#include "KDSoapMessageWriter_p.h"
#include <QNetworkInterface>

static bool isMulticastAddress(const QHostAddress &address) {
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        return address.isInSubnet(QHostAddress (QLatin1String("224.0.0.0")), 4);
    } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        return address.isInSubnet(QHostAddress (QLatin1String("ff00::")), 8);
    }
    return false;
}

KDSoapUdpClient::KDSoapUdpClient(QObject *parent) :
    QObject(parent),
    d_ptr(new KDSoapUdpClientPrivate(this))
{
    Q_D(KDSoapUdpClient);
    d->socketIPv4 = new QUdpSocket(this);
    connect(d->socketIPv4, SIGNAL(readyRead()), d, SLOT(readyRead()));
    d->socketIPv6 = new QUdpSocket(this);
    connect(d->socketIPv6, SIGNAL(readyRead()), d, SLOT(readyRead()));
}

KDSoapUdpClient::~KDSoapUdpClient()
{
    delete d_ptr;
}


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
bool KDSoapUdpClient::bind(quint16 port, QAbstractSocket::BindMode mode) {
    Q_D(KDSoapUdpClient);
    const QHostAddress AnyIPv4(QLatin1String("0.0.0.0"));
    bool rc = true;
    // Workaround for lack of dual stack sockets in Qt4
    // Qt5 supports binding to QHostAddress::Any, which will listen on both IPv4 and IPv6 interfaces.
    // TODO: use a single socket once we drop Qt4 support
    rc = d->socketIPv4->bind(AnyIPv4, port, mode) && rc;
    rc = d->socketIPv6->bind(QHostAddress::AnyIPv6, port, mode) && rc;
    return rc;
}
#else
bool KDSoapUdpClient::bind(quint16 port) {
    Q_D(KDSoapUdpClient);
    const QHostAddress AnyIPv4(QLatin1String("0.0.0.0"));
    bool rc = true;
    // Workaround for lack of dual stack sockets in Qt4
    // Qt5 supports binding to QHostAddress::Any, which will listen on both IPv4 and IPv6 interfaces.
    rc = d->socketIPv4->bind(AnyIPv4, port) && rc;
    rc = d->socketIPv6->bind(QHostAddress::AnyIPv6, port) && rc;
    return rc;
}
#endif

void KDSoapUdpClient::setSoapVersion(KDSoap::SoapVersion version) {
    Q_D(KDSoapUdpClient);
    d->soapVersion = version;
}

bool KDSoapUdpClient::sendMessage(const KDSoapMessage &message, const KDSoapHeaders &headers, const QHostAddress &address, quint16 port) {
    Q_D(KDSoapUdpClient);
    KDSoapMessageWriter msgWriter;
    msgWriter.setVersion(d->soapVersion);
    const QByteArray data = msgWriter.messageToXml(message, QString(), headers, QMap<QString, KDSoapMessage>());

    if (isMulticastAddress(address)) {
        bool anySuccess = false;
        const auto& allInterfaces = QNetworkInterface::allInterfaces();
        for (const auto &iface : allInterfaces) {
            if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
                iface.flags().testFlag(QNetworkInterface::CanMulticast)) {
                //qDebug() << "Sending multicast to" << iface.name() << address << ":" << data;
                if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                    d->socketIPv4->setMulticastInterface(iface);
                    qint64 writtenSize = d->socketIPv4->writeDatagram(data, address, port);
                    anySuccess = anySuccess || (writtenSize == data.size());
                } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
                    d->socketIPv6->setMulticastInterface(iface);
                    qint64 writtenSize = d->socketIPv6->writeDatagram(data, address, port);
                    anySuccess = anySuccess || (writtenSize == data.size());
                }
            }
        }
        return anySuccess;
    } else {
        //qDebug() << "Sending to" << address << ":" << data;
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            qint64 writtenSize = d->socketIPv4->writeDatagram(data, address, port);
            return writtenSize == data.size();
        } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
            qint64 writtenSize = d->socketIPv6->writeDatagram(data, address, port);
            return writtenSize == data.size();
        }
    }
    return false;
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
    //qDebug() << "Received datagram from:" << senderAddress << "data:" << QString::fromUtf8(messageData);

    KDSoapMessage replyMessage;
    KDSoapHeaders replyHeaders;

    KDSoapMessageReader reader;
    reader.xmlToMessage(messageData, &replyMessage, 0, &replyHeaders, soapVersion);

    emit q->receivedMessage(replyMessage, replyHeaders, senderAddress, senderPort);
}
