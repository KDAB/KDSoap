/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapClientInterface_p.h"
#include "KDSoapMessageWriter_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapValue.h"
#include <QDebug>
#include <QVariant>

KDSoapMessageWriter::KDSoapMessageWriter()
    : m_version(KDSoap::SOAP1_1)
{
}

void KDSoapMessageWriter::setVersion(KDSoap::SoapVersion version)
{
    m_version = version;
}

void KDSoapMessageWriter::setMessageNamespace(const QString &ns)
{
    m_messageNamespace = ns;
}

QByteArray KDSoapMessageWriter::messageToXml(const KDSoapMessage &message, const QString &method, const KDSoapHeaders &headers,
                                             const QMap<QString, KDSoapMessage> &persistentHeaders, const KDSoapAuthentication &authentication) const
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.writeStartDocument();

    KDSoapNamespacePrefixes namespacePrefixes;
    namespacePrefixes.writeStandardNamespaces(writer, m_version, message.hasMessageAddressingProperties(),
                                              message.messageAddressingProperties().addressingNamespace());

    QString soapEnvelope;
    QString soapEncoding;
    if (m_version == KDSoap::SOAP1_1) {
        soapEnvelope = KDSoapNamespaceManager::soapEnvelope();
        soapEncoding = KDSoapNamespaceManager::soapEncoding();
    } else if (m_version == KDSoap::SOAP1_2) {
        soapEnvelope = KDSoapNamespaceManager::soapEnvelope200305();
        soapEncoding = KDSoapNamespaceManager::soapEncoding200305();
    }

    writer.writeStartElement(soapEnvelope, QLatin1String("Envelope"));

    // This has been removed, see https://msdn.microsoft.com/en-us/library/ms995710.aspx for details
    // writer.writeAttribute(soapEnvelope, QLatin1String("encodingStyle"), soapEncoding);

    QString messageNamespace = m_messageNamespace;
    if (!message.namespaceUri().isEmpty() && messageNamespace != message.namespaceUri()) {
        messageNamespace = message.namespaceUri();
    }

    if (!headers.isEmpty() || !persistentHeaders.isEmpty() || message.hasMessageAddressingProperties() || authentication.hasWSUsernameTokenHeader()) {
        // This writeNamespace line adds the xmlns:n1 to <Envelope>, which looks ugly and unusual (and breaks all unittests)
        // However it's the best solution in case of headers, otherwise we get n1 in the header and n2 in the body,
        // and xsi:type attributes that refer to n1, which isn't defined in the body...
        namespacePrefixes.writeNamespace(writer, messageNamespace, QLatin1String("n1") /*make configurable?*/);
        writer.writeStartElement(soapEnvelope, QLatin1String("Header"));
        for (const KDSoapMessage &header : std::as_const(persistentHeaders)) {
            header.writeChildren(namespacePrefixes, writer, header.use(), messageNamespace, true);
        }
        for (const KDSoapMessage &header : std::as_const(headers)) {
            header.writeChildren(namespacePrefixes, writer, header.use(), messageNamespace, true);
        }
        if (message.hasMessageAddressingProperties()) {
            message.messageAddressingProperties().writeMessageAddressingProperties(namespacePrefixes, writer, messageNamespace, true);
        }
        if (authentication.hasWSUsernameTokenHeader()) {
            authentication.writeWSUsernameTokenHeader(writer);
        }
        writer.writeEndElement(); // Header
    } else {
        // So in the standard case (no headers) we just rely on Qt calling it n1 and insert it into the map.
        // Calling this after the writeStartElement(ns, elementName) below leads to a double-definition of n1.
        namespacePrefixes.insert(messageNamespace, QString::fromLatin1("n1"));
    }

    writer.writeStartElement(soapEnvelope, QLatin1String("Body"));

    const QString elementName = !method.isEmpty() ? method : message.name();
    if (elementName.isEmpty()) {
        if (message.isNil()) {
            // null message, ok (e.g. no arguments, in document/literal mode)
        } else {
            qWarning("ERROR: Non-empty message with an empty name!");
            qDebug() << message;
        }
    } else {
        // Note that the message itself is always qualified.
        // isQualified() is only for child elements.
        if (!message.isFault()) {
            writer.writeStartElement(messageNamespace, elementName);
        } else {
            // Fault element should be inside soap namespace
            writer.writeStartElement(soapEnvelope, elementName);
        }
        message.writeElementContents(namespacePrefixes, writer, message.use(), messageNamespace);
        writer.writeEndElement();
    }
    writer.writeEndElement(); // Body
    writer.writeEndElement(); // Envelope
    writer.writeEndDocument();

    return data;
}
