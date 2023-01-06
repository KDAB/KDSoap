/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPMESSAGEWRITER_P_H
#define KDSOAPMESSAGEWRITER_P_H

#include "KDSoapAuthentication.h"
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include <QtCore/QByteArray>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QXmlStreamWriter>
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

    QByteArray messageToXml(const KDSoapMessage &message, const QString &method /*empty in document style*/,
                            const KDSoapHeaders &headers,
                            const QMap<QString, KDSoapMessage> &persistentHeaders,
                            const KDSoapAuthentication &authentication = KDSoapAuthentication()) const;

private:
    QString m_messageNamespace;
    KDSoap::SoapVersion m_version;
};

#endif // KDSOAPMESSAGEWRITER_P_H
