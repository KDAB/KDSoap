/****************************************************************************
** Copyright (C) 2001-2011 Klaralvdalens Datakonsult AB.  All rights reserved.
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
#include "KDSoapMessageWriter_p.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapValue.h"
#include <QVariant>
#include <QDebug>

KDSoapMessageWriter::KDSoapMessageWriter()
{
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
    namespacePrefixes.writeStandardNamespaces(writer);

    const QString soapNS = KDSoapNamespaceManager::soapEnvelope();
    writer.writeStartElement(soapNS, QLatin1String("Envelope"));
    writer.writeAttribute(soapNS, QLatin1String("encodingStyle"), KDSoapNamespaceManager::soapEncoding());

    if (!headers.isEmpty() || !persistentHeaders.isEmpty()) {
        // This writeNamespace line adds the xmlns:n1 to <Envelope>, which looks ugly and unusual (and breaks all unittests)
        // However it's the best solution in case of headers, otherwise we get n1 in the header and n2 in the body,
        // and xsi:type attributes that refer to n1, which isn't defined in the body...
        namespacePrefixes.writeNamespace(writer, m_messageNamespace, QLatin1String("n1") /*make configurable?*/);
        writer.writeStartElement(soapNS, QLatin1String("Header"));
        Q_FOREACH(const KDSoapMessage& header, persistentHeaders) {
            header.writeChildren(namespacePrefixes, writer, header.use(), m_messageNamespace);
        }
        Q_FOREACH(const KDSoapMessage& header, headers) {
            header.writeChildren(namespacePrefixes, writer, header.use(), m_messageNamespace);
        }
        writer.writeEndElement(); // Header
    } else {
        // So in the standard case (no headers) we just rely on Qt calling it n1 and insert it into the map.
        // Calling this after the writeStartElement(method) below leads to a double-definition of n1.
        namespacePrefixes.insert(m_messageNamespace, QString::fromLatin1("n1"));
    }

    writer.writeStartElement(soapNS, QLatin1String("Body"));

    if (!method.isEmpty())
        writer.writeStartElement(m_messageNamespace, method);
    else
        writer.writeStartElement(m_messageNamespace, message.name());
    message.writeElementContents(namespacePrefixes, writer, message.use(), m_messageNamespace);

    writer.writeEndElement(); // <method>

    writer.writeEndElement(); // Body
    writer.writeEndElement(); // Envelope
    writer.writeEndDocument();

    if (qgetenv("KDSOAP_DEBUG").toInt()) {
        qDebug() << data;
    }
    return data;
}
