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
#ifndef KDSOAPSERVERSOCKET_P_H
#define KDSOAPSERVERSOCKET_P_H

#include <QtGlobal>

#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#else
#include <QTcpSocket>
#endif

#include <QMap>
QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE
class KDSoapSocketList;
class KDSoapServerObjectInterface;
class KDSoapMessage;
class KDSoapHeaders;

class KDSoapServerSocket
#ifndef QT_NO_OPENSSL
        : public QSslSocket
#else
        : public QTcpSocket
#endif
{
    Q_OBJECT
public:
    KDSoapServerSocket(KDSoapSocketList* owner, QObject* serverObject);
    ~KDSoapServerSocket();

    void setResponseDelayed();
    void sendDelayedReply(KDSoapServerObjectInterface* serverObjectInterface, const KDSoapMessage& replyMsg);
    void sendReply(KDSoapServerObjectInterface* serverObjectInterface, const KDSoapMessage& replyMsg);
Q_SIGNALS:
    void socketDeleted(KDSoapServerSocket *);

private Q_SLOTS:
    void slotReadyRead();

private:
    void handleRequest(const QMap<QByteArray, QByteArray> &headers, const QByteArray &receivedData);
    bool handleWsdlDownload();
    bool handleFileDownload(KDSoapServerObjectInterface* serverObjectInterface, const QString& path);
    void makeCall(KDSoapServerObjectInterface* serverObjectInterface,
                  const KDSoapMessage& requestMsg, KDSoapMessage& replyMsg,
                  const KDSoapHeaders& requestHeaders,
                  const QByteArray& soapAction, const QString &path);
    void handleError(KDSoapMessage& replyMsg, const char* errorCode, const QString& error);
    void setSocketEnabled(bool enabled);
    void writeXML(const QByteArray &xmlResponse, bool isFault);
    friend class KDSoapServerObjectInterface;

    KDSoapSocketList* m_owner;
    QObject* m_serverObject;
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
