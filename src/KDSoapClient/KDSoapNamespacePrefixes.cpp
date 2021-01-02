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
#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapNamespaceManager.h"

void KDSoapNamespacePrefixes::writeStandardNamespaces(QXmlStreamWriter &writer,
        KDSoap::SoapVersion version,
        bool messageAddressingEnabled,
        KDSoapMessageAddressingProperties::KDSoapAddressingNamespace messageAddressingNamespace)
{
    if (version == KDSoap::SOAP1_1) {
        writeNamespace(writer, KDSoapNamespaceManager::soapEnvelope(), QLatin1String("soap"));
        writeNamespace(writer, KDSoapNamespaceManager::soapEncoding(), QLatin1String("soap-enc"));
    } else if (version == KDSoap::SOAP1_2) {
        writeNamespace(writer, KDSoapNamespaceManager::soapEnvelope200305(), QLatin1String("soap"));
        writeNamespace(writer, KDSoapNamespaceManager::soapEncoding200305(), QLatin1String("soap-enc"));
    }

    writeNamespace(writer, KDSoapNamespaceManager::xmlSchema2001(), QLatin1String("xsd"));
    writeNamespace(writer, KDSoapNamespaceManager::xmlSchemaInstance2001(), QLatin1String("xsi"));

    if (messageAddressingEnabled) {
        const QString addressingNS = KDSoapMessageAddressingProperties::addressingNamespaceToString(messageAddressingNamespace);
        writeNamespace(writer, addressingNS, QLatin1String("wsa"));
    }

    // Also insert known variants
    insert(KDSoapNamespaceManager::xmlSchema1999(), QString::fromLatin1("xsd"));
    insert(KDSoapNamespaceManager::xmlSchemaInstance1999(), QString::fromLatin1("xsi"));
}
