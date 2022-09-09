/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2020-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef KDSOAPUDPCLIENT_H
#define KDSOAPUDPCLIENT_H

#include "KDSoapClientInterface.h"
#include "KDSoapGlobal.h"

#include <QAbstractSocket>
#include <QObject>

class KDSoapHeaders;
class KDSoapMessage;
class KDSoapUdpClientPrivate;
QT_BEGIN_NAMESPACE
class QHostAddress;
QT_END_NAMESPACE

/**
 * \brief KDSoapUdpClient provides an interface for implementing a
 * [SOAP-over-UDP](https://docs.oasis-open.org/ws-dd/soapoverudp/1.1/os/wsdd-soapoverudp-1.1-spec-os.html) client.
 *
 * One-way SOAP-over-UDP can be send by simply using sendMessage().
 *
 * Request-response SOAP-over-UDP is supported by bind()ing to a sender UDP
 * port.  You can send the request using sendMessage() and the response will is
 * signaled using receivedMessage(). receivedMessage() will signal any response,
 * including those of other requests, there is no help with finding the correct
 * response. The WS-Addressing properties of the message can be used to filter
 * the received responses.
 *
 * \code
 * auto soapUdpClient = new KDSoapUdpClient(this);
 * connect(soapUdpClient, &KDSoapUdpClient::receivedMessage, [=](const KDSoapMessage &message, const KDSoapHeaders &headers, const QHostAddress
 * &address, quint16 port) { if(message.messageAddressingProperties().action() ==
 * QStringLiteral("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches")) { TNS__ProbeMatchesType probeMatches;
 *     probeMatches.deserialize(message);
 *     qDebug() << "Received probe match from" << address;
 *   }
 * });
 * soapUdpClient->bind(3702);
 *
 * TNS__ProbeType probe;
 *
 * KDSoapMessage message;
 * message = probe.serialize(QStringLiteral("Probe"));
 * message.setUse(KDSoapMessage::LiteralUse);
 * message.setNamespaceUri(QStringLiteral("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01"));
 *
 * KDSoapMessageAddressingProperties addressing;
 * addressing.setAction(QStringLiteral("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe"));
 * addressing.setMessageID(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString(QUuid::WithoutBraces)); // WithoutBraces requires Qt 5.11
 * addressing.setDestination(QStringLiteral("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01"));
 * addressing.setReplyEndpointAddress(KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous));
 * message.setMessageAddressingProperties(addressing);
 *
 * soapUdpClient->sendMessage(message, KDSoapHeaders(), QHostAddress("239.255.255.250"), 3702);
 * \endcode
 *
 * \since 1.9
 */
class KDSOAP_EXPORT KDSoapUdpClient : public QObject
{
    Q_OBJECT

public:
    explicit KDSoapUdpClient(QObject *parent = nullptr);

    ~KDSoapUdpClient();

    /**
     * Bind UDP socket to port. This is needed to receive messages. Both the
     * IPv4 and IPv6 port will be bound.
     * \param port The UDP port to bind to. When port is 0, a random port is chosen.
     * \param mode This is passed directly to QUdpSocket::bind().
     * \see receivedMessage()
     * \since 1.9
     */
    bool bind(quint16 port = 0, QAbstractSocket::BindMode mode = QAbstractSocket::DefaultForPlatform);
    /**
     * Sets the SOAP version to be used for any subsequent send message.
     * \param version \a SOAP1_1 or \a SOAP1_2
     * The default version is SOAP 1.2.
     */
    void setSoapVersion(KDSoap::SoapVersion version);

public Q_SLOTS:
    /**
     * Send a SOAP-over-UDP message to IP address.
     * \param message  The actual message to be send. Use
     * KDSoapMessage::setMessageAddressingProperties() to set the WS-Addressing
     * properties of the message.
     * \param headers can be used to add additional SOAP headers.
     * \param address The address to send to message to. Messages send to a
     * multicast address will be send to all interfaces.
     * \param port The UDP port to send the message to
     * \since 1.9
     */
    bool sendMessage(const KDSoapMessage &message, const KDSoapHeaders &headers, const QHostAddress &address, quint16 port);

Q_SIGNALS:
    /**
     * emitted when a SOAP-over-UDP message is received over a bound socket.
     * KDSoapUdpClient doesn't do any filtering, so duplicate messages, spoofed
     * or responses to other requests will all be received. For example, if a
     * message is send via both IPv4 and IPv6, then the receivedMessage will be
     * emitted twice (with the same message, but with a different address)
     * \param message The parsed message received over the socket. Use
     * KDSoapMessage::messageAddressingProperties() to see who the recipient is
     * and what SOAP action is requested.
     * \param headers The additional headers of the message
     * \param address The IP-address of the sender
     * \param port The UDP port of the sender
     * \see bind()
     * \since 1.9
     */
    void receivedMessage(const KDSoapMessage &message, const KDSoapHeaders &headers, const QHostAddress &address, quint16 port);

private:
    KDSoapUdpClientPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(KDSoapUdpClient)
};

#endif // KDSOAPUDPCLIENT_H
