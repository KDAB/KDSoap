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
#ifndef KDSOAPMESSAGEWRITER_P_H
#define KDSOAPMESSAGEWRITER_P_H

#include "KDSoapMessage.h"
#include "KDSoapAuthentication.h"
#include "KDSoapClientInterface.h"
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QMap>
class KDSoapMessage;
class KDSoapHeaders;
class KDSoapNamespacePrefixes;
class KDSoapValue;
class KDSoapValueList;

/**
 * \internal
 * Internal class -- only exported for the server lib
 */
class KDSOAP_EXPORT KDSoapMessageWriter
{
public:
    KDSoapMessageWriter();

    void setVersion(KDSoap::SoapVersion version);
    void setMessageNamespace(const QString &ns);

    QByteArray messageToXml(const KDSoapMessage &message, const QString &method /*empty in document style*/, const KDSoapHeaders &headers,
                            const QMap<QString, KDSoapMessage> &persistentHeaders,
                            const KDSoapAuthentication &authentication = KDSoapAuthentication()) const;

private:
    QString m_messageNamespace;
    KDSoap::SoapVersion m_version;
};

#endif // KDSOAPMESSAGEWRITER_P_H
