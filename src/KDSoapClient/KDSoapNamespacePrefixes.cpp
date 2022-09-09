/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapClientInterface_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapNamespacePrefixes_p.h"

void KDSoapNamespacePrefixes::writeStandardNamespaces(QXmlStreamWriter &writer, KDSoap::SoapVersion version, bool messageAddressingEnabled,
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
