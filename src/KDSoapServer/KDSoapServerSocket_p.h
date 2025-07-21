/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPSERVERSOCKET_P_H
#define KDSOAPSERVERSOCKET_P_H

#include <QtGlobal>

#include <QTcpSocket> //may define QT_NO_SSL
#ifndef QT_NO_SSL
#include <QSslSocket>
#endif

#include <KDSoapClient/KDSoapClientInterface.h>
#include <QMap>
QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE
class KDSoapSocketList;
class KDSoapServerObjectInterface;
class KDSoapMessage;
class KDSoapHeaders;

class KDSoapServerSocket
#ifndef QT_NO_SSL
    : public QSslSocket
#else
    : public QTcpSocket
#endif
{
    Q_OBJECT
public:
    KDSoapServerSocket(KDSoapSocketList *owner, QObject *serverObject);
    ~KDSoapServerSocket();

    // EncodingFormat represents recognized HTTP Content-Encoding values.
    // See RFC 9110 §8.4.1: https://datatracker.ietf.org/doc/html/rfc9110#section-8.4.1
    enum EncodingFormat
    { // Note: deliberately lowercase for matching purposes
        identity = 0x1, // No compression
        gzip = 0x2, // Gzip - RFC 9110 §8.4.2.1 (uses RFC 1952 format), NOT supported by rcc, supported by QNetworkAccessManager
        deflate = 0x4, // Zlib - RFC 9110 §8.4.2.2 (uses RFC 1950 format), supported by rcc, supported by QNetworkAccessManager
        br = 0x8, // Brotli - RFC 7932, NOT supported by rcc, supported by QNetworkAccessManager if built with Brotli (Qt >= 5.10)
        zstd = 0x10 // Zstd - RFC 8878, supported by rcc, NOT supported by QNetworkAccessManager
    };
    Q_DECLARE_FLAGS(EncodingFormats, EncodingFormat)
    Q_FLAG(EncodingFormats)

    void setResponseDelayed();
    void sendDelayedReply(KDSoapServerObjectInterface *serverObjectInterface, const KDSoapMessage &replyMsg);
    void sendReply(KDSoapServerObjectInterface *serverObjectInterface, const KDSoapMessage &replyMsg);
Q_SIGNALS:
    void socketDeleted(KDSoapServerSocket *);

private Q_SLOTS:
    void slotReadyRead();

private:
    void handleRequest(const QMap<QByteArray, QByteArray> &headers, const QByteArray &receivedData);
    bool handleWsdlDownload(KDSoapServerObjectInterface *serverObjectInterface);
    bool handleFileDownload(KDSoapServerObjectInterface *serverObjectInterface, const QString &path);
    void makeCall(KDSoapServerObjectInterface *serverObjectInterface, const KDSoapMessage &requestMsg, KDSoapMessage &replyMsg,
                  const KDSoapHeaders &requestHeaders, const QByteArray &soapAction, const QString &path, KDSoap::SoapVersion soapVersion);
    void handleError(KDSoapMessage &replyMsg, const char *errorCode, const QString &error, KDSoap::SoapVersion soapVersion = KDSoap::SoapVersion::SOAP1_1);
    void setSocketEnabled(bool enabled);
    void writeXML(const QByteArray &xmlResponse, bool isFault, KDSoap::SoapVersion soapVersion);
    friend class KDSoapServerObjectInterface;

    KDSoapSocketList *m_owner;
    QObject *m_serverObject;
    bool m_delayedResponse;
    bool m_doDebug;
    bool m_socketEnabled;
    bool m_receivedData;

    // Current request being assembled
    bool m_useRawXML;
    int m_bytesReceived;
    int m_chunkStart;
    QMap<QByteArray, QByteArray> m_httpHeaders;
    QByteArray m_requestBuffer;
    QByteArray m_decodedRequestBuffer; // used for chunked transfer encoding only

    // Data for the current call (stored here for delayed replies)
    QString m_messageNamespace;
    QString m_method;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDSoapServerSocket::EncodingFormats)

#endif // KDSOAPSERVERSOCKET_P_H
