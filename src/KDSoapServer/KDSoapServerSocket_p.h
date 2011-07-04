/****************************************************************************
** Copyright (C) 2010-2011 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL.txt included.
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
class QObject;
class KDSoapSocketList;
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

private Q_SLOTS:
    void slotReadyRead();

private:
    void makeCall(const KDSoapMessage& requestMsg, KDSoapMessage& replyMsg,
                  const KDSoapHeaders& requestHeaders, KDSoapHeaders& responseHeaders,
                  const QByteArray& soapAction);
    void handleError(KDSoapMessage& replyMsg, const char* errorCode, const QString& error);

    KDSoapSocketList* m_owner;
    QObject* m_serverObject;
    bool m_doDebug;
    QByteArray m_requestBuffer;
};

#endif // KDSOAPSERVERSOCKET_P_H
