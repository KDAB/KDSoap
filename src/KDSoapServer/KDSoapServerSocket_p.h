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

    struct FileRange {
        enum class Type {
            InvalidRange,
            FullFile,
            ValidRanges,
        } type;
        QVector<QPair<int,int>> ranges = {};
    };
    FileRange determineFileRanges(int fileSize) const;

    void writeFileRanges(QIODevice *device, const QVector<QPair<int, int>> &requestedRanges, const QByteArray &contentType, const QByteArray &additionalHttpHeaders);
    bool handleFileDownload(KDSoapServerObjectInterface *serverObjectInterface, const QString &path);
    void makeCall(KDSoapServerObjectInterface *serverObjectInterface, const KDSoapMessage &requestMsg, KDSoapMessage &replyMsg,
                  const KDSoapHeaders &requestHeaders, const QByteArray &soapAction, const QString &path, KDSoap::SoapVersion soapVersion);
    void handleError(KDSoapMessage &replyMsg, const char *errorCode, const QString &error, KDSoap::SoapVersion soapVersion = KDSoap::SoapVersion::SOAP1_1);
    void setSocketEnabled(bool enabled);
    void writeXML(KDSoapServerObjectInterface *serverObjectInterface, const QByteArray &xmlResponse, bool isFault);
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

#endif // KDSOAPSERVERSOCKET_P_H
