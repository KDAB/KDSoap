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
#include "KDSoapMessageWriter_p.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapValue.h"
#include <QVariant>
#include <QDebug>

KDSoapMessageWriter::KDSoapMessageWriter()
    : m_version(KDSoapClientInterface::SOAP1_1)
{
}

void KDSoapMessageWriter::setVersion(KDSoapClientInterface::SoapVersion version)
{
    m_version = version;
}

void KDSoapMessageWriter::setMessageNamespace(const QString &ns)
{
    m_messageNamespace = ns;
}

QByteArray KDSoapMessageWriter::messageToXml(const KDSoapMessage& message, const QString& method,
                                             const KDSoapHeaders& headers, const QMap<QString, KDSoapMessage>& persistentHeaders) const
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.writeStartDocument();

    KDSoapNamespacePrefixes namespacePrefixes;
    namespacePrefixes.writeStandardNamespaces(writer, m_version);

    QString soapEnvelope;
    QString soapEncoding;
    if (m_version == KDSoapClientInterface::SOAP1_1) {
        soapEnvelope = KDSoapNamespaceManager::soapEnvelope();
        soapEncoding = KDSoapNamespaceManager::soapEncoding();
    } else if (m_version == KDSoapClientInterface::SOAP1_2) {
        soapEnvelope = KDSoapNamespaceManager::soapEnvelope200305();
        soapEncoding = KDSoapNamespaceManager::soapEncoding200305();
    }

    writer.writeStartElement(soapEnvelope, QLatin1String("Envelope"));
    writer.writeAttribute(soapEnvelope, QLatin1String("encodingStyle"), soapEncoding);

    QString messageNamespace = m_messageNamespace;
    if (!message.namespaceUri().isEmpty() && messageNamespace != message.namespaceUri()) {
        messageNamespace = message.namespaceUri();
    }

    if (!headers.isEmpty() || !persistentHeaders.isEmpty()) {
        // This writeNamespace line adds the xmlns:n1 to <Envelope>, which looks ugly and unusual (and breaks all unittests)
        // However it's the best solution in case of headers, otherwise we get n1 in the header and n2 in the body,
        // and xsi:type attributes that refer to n1, which isn't defined in the body...
        namespacePrefixes.writeNamespace(writer, messageNamespace, QLatin1String("n1") /*make configurable?*/);
        writer.writeStartElement(soapEnvelope, QLatin1String("Header"));
        Q_FOREACH(const KDSoapMessage& header, persistentHeaders) {
            header.writeChildren(namespacePrefixes, writer, header.use(), messageNamespace, true);
        }
        Q_FOREACH(const KDSoapMessage& header, headers) {
            header.writeChildren(namespacePrefixes, writer, header.use(), messageNamespace, true);
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
        if (message.isNull()) {
            // null message, ok (e.g. no arguments, in document/literal mode)
        } else {
            qWarning("ERROR: Non-empty message with an empty name!");
            qDebug() << message;
        }
    } else {
        // Note that the message itself is always qualified.
        // http://www.ibm.com/developerworks/webservices/library/ws-tip-namespace/index.html
        // isQualified() is only for child elements.
        writer.writeStartElement(messageNamespace, elementName);
        message.writeElementContents(namespacePrefixes, writer, message.use(), messageNamespace);
        writer.writeEndElement();
    }

    writer.writeEndElement(); // Body
    writer.writeEndElement(); // Envelope
    writer.writeEndDocument();

    if (qgetenv("KDSOAP_DEBUG").toInt()) {
        qDebug() << data;
    }
    return data;
}
