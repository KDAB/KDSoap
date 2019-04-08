/****************************************************************************
** Copyright (C) 2010-2019 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

    QByteArray messageToXml(const KDSoapMessage &message, const QString &method /*empty in document style*/,
                            const KDSoapHeaders &headers,
                            const QMap<QString, KDSoapMessage> &persistentHeaders,
                            const KDSoapAuthentication &authentication = KDSoapAuthentication()) const;

private:
    QString m_messageNamespace;
    KDSoap::SoapVersion m_version;

};

#endif // KDSOAPMESSAGEWRITER_P_H
