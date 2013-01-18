/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
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
    QNetworkAccessManager m_accessManager;
    QString m_endPoint;
    QString m_messageNamespace;
    KDSoapClientThread m_thread;
    KDSoapAuthentication m_authentication;
    QMap<QString, KDSoapMessage> m_persistentHeaders;
    KDSoapClientInterface::SoapVersion m_version;
    KDSoapClientInterface::Style m_style;
    bool m_ignoreSslErrors;
    KDSoapHeaders m_lastResponseHeaders;
#ifndef QT_NO_OPENSSL
    QSslConfiguration m_sslConfiguration;
#endif
    KDSoapSslHandler* m_sslHandler;

    QNetworkRequest prepareRequest(const QString &method, const QString& action);
    QBuffer* prepareRequestBuffer(const QString& method, const KDSoapMessage& message, const KDSoapHeaders& headers);
    void writeElementContents(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValue& element, KDSoapMessage::Use use);
    void writeChildren(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValueList& args, KDSoapMessage::Use use);
    void writeAttributes(QXmlStreamWriter& writer, const QList<KDSoapValue>& attributes);
    void setupReply(QNetworkReply* reply);

private Q_SLOTS:
    void _kd_slotAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);
};

#endif // KDSOAPCLIENTINTERFACE_P_H
