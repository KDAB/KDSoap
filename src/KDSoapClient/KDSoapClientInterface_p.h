/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
#ifndef KDSOAPCLIENTINTERFACE_P_H
#define KDSOAPCLIENTINTERFACE_P_H

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookieJar>
#include <QtCore/QXmlStreamWriter>

#include "KDSoapClientInterface.h"
#include "KDSoapClientThread_p.h"
#include "KDSoapAuthentication.h"
QT_BEGIN_NAMESPACE
class QBuffer;
QT_END_NAMESPACE
class KDSoapMessage;
class KDSoapNamespacePrefixes;

class KDSoapClientInterfacePrivate : public QObject
{
    Q_OBJECT
public:
    KDSoapClientInterfacePrivate();
    ~KDSoapClientInterfacePrivate();

    // Warning: this accessManager is only used by asyncCall and callNoReply.
    // For blocking calls, the thread has its own accessManager.
    QNetworkAccessManager *m_accessManager;
    QString m_endPoint;
    QString m_messageNamespace;
    KDSoapClientThread m_thread;
    KDSoapAuthentication m_authentication;
    QMap<QString, KDSoapMessage> m_persistentHeaders;
    QMap<QByteArray, QByteArray> m_httpHeaders;
    KDSoap::SoapVersion m_version;
    KDSoapClientInterface::Style m_style;
    bool m_ignoreSslErrors;
    KDSoapHeaders m_lastResponseHeaders;
#ifndef QT_NO_SSL
    QList<QSslError> m_ignoreErrorsList;
    QSslConfiguration m_sslConfiguration;
    KDSoapSslHandler *m_sslHandler;
#endif
    int m_timeout;

    QNetworkAccessManager *accessManager();
    QNetworkRequest prepareRequest(const QString &method, const QString &action);
    QBuffer *prepareRequestBuffer(const QString &method, const KDSoapMessage &message, const KDSoapHeaders &headers);
    void writeElementContents(KDSoapNamespacePrefixes &namespacePrefixes, QXmlStreamWriter &writer, const KDSoapValue &element, KDSoapMessage::Use use);
    void writeChildren(KDSoapNamespacePrefixes &namespacePrefixes, QXmlStreamWriter &writer, const KDSoapValueList &args, KDSoapMessage::Use use);
    void writeAttributes(QXmlStreamWriter &writer, const QList<KDSoapValue> &attributes);
    void setupReply(QNetworkReply *reply);

private Q_SLOTS:
    void _kd_slotAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
};

#endif // KDSOAPCLIENTINTERFACE_P_H
