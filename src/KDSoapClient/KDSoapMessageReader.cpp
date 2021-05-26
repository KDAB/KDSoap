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

#include "KDSoapMessageReader_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDDateTime.h"

#include <QDebug>
#include <QXmlStreamReader>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define QStringView QStringRef
#endif

static QStringView namespaceForPrefix(const QXmlStreamNamespaceDeclarations &decls, const QString &prefix)
{
    for (const QXmlStreamNamespaceDeclaration &decl : qAsConst(decls)) {
        if (decl.prefix() == prefix) {
            return decl.namespaceUri();
        }
    }
    return QStringView();
}

static int xmlTypeToMetaType(const QString &xmlType)
{
    // Reverse operation from variantToXmlType in KDSoapClientInterface, keep in sync
    static const struct
    {
        const char *xml; // xsd: prefix assumed
        const int metaTypeId;
    } s_types[] = { { "string", QVariant::String }, // or QUrl
                    { "base64Binary", QVariant::ByteArray }, { "int", QVariant::Int }, // or long, or uint, or longlong
                    { "unsignedInt", QVariant::ULongLong },  { "boolean", QVariant::Bool }, { "float", QMetaType::Float },
                    { "double", QVariant::Double },          { "time", QVariant::Time },    { "date", QVariant::Date } };
    // Speed: could be sorted and then we could use qBinaryFind
    for (const auto &type : s_types) {
        if (xmlType == QLatin1String(type.xml)) {
            return type.metaTypeId;
        }
    }
    if (xmlType == QLatin1String("dateTime")) {
        return qMetaTypeId<KDDateTime>();
    }
    // This will happen with any custom type, don't bother the user
    // qDebug() << QString::fromLatin1("xmlTypeToMetaType: XML type %1 is not supported in "
    //                                "KDSoap, see the documentation").arg(xmlType);
    return -1;
}

static KDSoapValue parseElement(QXmlStreamReader &reader, const QXmlStreamNamespaceDeclarations &envNsDecls)
{
    const QXmlStreamNamespaceDeclarations combinedNamespaceDeclarations = envNsDecls + reader.namespaceDeclarations();
    const QString name = reader.name().toString();
    KDSoapValue val(name, QVariant());
    val.setNamespaceUri(reader.namespaceUri().toString());
    val.setNamespaceDeclarations(reader.namespaceDeclarations());
    val.setEnvironmentNamespaceDeclarations(combinedNamespaceDeclarations);
    // qDebug() << "parsing" << name;
    QVariant::Type metaTypeId = QVariant::Invalid;

    const QXmlStreamAttributes attributes = reader.attributes();
    for (const QXmlStreamAttribute &attribute : attributes) {
        const QStringView name = attribute.name();
        const QStringView ns = attribute.namespaceUri();
        const QStringView attrValue = attribute.value();
        // Parse xsi:type and soap-enc:arrayType
        // and ignore anything else from the xsi or soap-enc namespaces until someone needs it...
        if (ns == KDSoapNamespaceManager::xmlSchemaInstance1999() || ns == KDSoapNamespaceManager::xmlSchemaInstance2001()) {
            if (name == QLatin1String("type")) {
                // The type can be like xsd:float, resolve that
                const QString type = attrValue.toString();
                const int pos = type.indexOf(QLatin1Char(':'));
                const QString dataType = type.mid(pos + 1);
                val.setType(namespaceForPrefix(combinedNamespaceDeclarations, type.left(pos)).toString(), dataType);
                metaTypeId = static_cast<QVariant::Type>(xmlTypeToMetaType(dataType));
            }
            continue;
        } else if (ns == KDSoapNamespaceManager::soapEncoding() || ns == KDSoapNamespaceManager::soapEncoding200305()
                   || ns == KDSoapNamespaceManager::soapEnvelope() || ns == KDSoapNamespaceManager::soapEnvelope200305()) {
            continue;
        }
        // qDebug() << "Got attribute:" << name << ns << "=" << attrValue;
        val.childValues().attributes().append(KDSoapValue(name.toString(), attrValue.toString()));
    }
    QString text;
    while (reader.readNext() != QXmlStreamReader::Invalid) {
        if (reader.isEndElement()) {
            break;
        }
        if (reader.isCharacters()) {
            text = reader.text().toString();
            // qDebug() << "text=" << text;
        } else if (reader.isStartElement()) {
            const KDSoapValue subVal = parseElement(reader, combinedNamespaceDeclarations); // recurse
            val.childValues().append(subVal);
        }
    }

    if (!text.isEmpty()) {
        QVariant variant(text);
        // qDebug() << text << variant << metaTypeId;
        // With use=encoded, we have type info, we can convert the variant here
        // Otherwise, for servers, we do it later, once we know the method's parameter types.
        if (metaTypeId != QVariant::Invalid) {
            QVariant copy = variant;
            if (!variant.convert(metaTypeId)) {
                variant = copy;
            }
        }
        val.setValue(variant);
    }
    return val;
}

KDSoapMessageReader::KDSoapMessageReader()
{
}

static bool isInvalidCharRef(const QByteArray &charRef)
{
    bool ok = true;
    int symbol = charRef.indexOf('x');
    int end = charRef.indexOf(';');

    if (symbol == -1 || end == -1) {
        return false;
    }

    uint val = charRef.mid(symbol + 1, end - symbol - 1).toInt(&ok, 16);

    if (!ok) {
        return false;
    }

    if (val != 0x9 && val != 0xa && val != 0xd && (val <= 0x20)) {
        return true;
    }

    return false;
}

static QByteArray handleNotWellFormedError(const QByteArray &data, qint64 offset)
{
    qint64 i = offset - 1; // offset is the char following the failing one
    QByteArray dataCleanedUp;
    QByteArray originalSequence;

    while (i >= 0 && data.at(i) != '&') {
        if (data.at(i) == '<') { // InvalidXML but not invalid characters related
            return dataCleanedUp;
        }

        originalSequence.prepend(data.at(i));
        i--;
    }

    if (isInvalidCharRef(originalSequence)) {
        qWarning() << "found an invalid character sequence to remove:" << QLatin1String(originalSequence.prepend('&').constData());
        dataCleanedUp = data;
        dataCleanedUp = dataCleanedUp.replace(originalSequence, "?");
    }
    return dataCleanedUp;
}

KDSoapMessageReader::XmlError KDSoapMessageReader::xmlToMessage(const QByteArray &data, KDSoapMessage *pMsg, QString *pMessageNamespace,
                                                                KDSoapHeaders *pRequestHeaders, KDSoap::SoapVersion soapVersion) const
{
    Q_ASSERT(pMsg);
    QXmlStreamReader reader(data);
    if (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("Envelope")
            && (reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()
                || reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope200305())) {
            const QXmlStreamNamespaceDeclarations envNsDecls = reader.namespaceDeclarations();
            if (reader.readNextStartElement()) {
                if (reader.name() == QLatin1String("Header")
                    && (reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()
                        || reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope200305())) {
                    KDSoapMessageAddressingProperties messageAddressingProperties;
                    while (reader.readNextStartElement()) {
                        if (KDSoapMessageAddressingProperties::isWSAddressingNamespace(reader.namespaceUri().toString())) {
                            KDSoapValue value = parseElement(reader, envNsDecls);
                            messageAddressingProperties.readMessageAddressingProperty(value);
                        } else {
                            KDSoapMessage header;
                            static_cast<KDSoapValue &>(header) = parseElement(reader, envNsDecls);
                            pRequestHeaders->append(header);
                        }
                    }
                    pMsg->setMessageAddressingProperties(messageAddressingProperties);
                    reader.readNextStartElement(); // read <Body>
                }
                if (reader.name() == QLatin1String("Body")
                    && (reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()
                        || reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope200305())) {
                    if (reader.readNextStartElement()) {
                        *pMsg = parseElement(reader, envNsDecls);
                        if (pMessageNamespace) {
                            *pMessageNamespace = pMsg->namespaceUri();
                        }
                        if (pMsg->name() == QLatin1String("Fault")
                            && (reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope()
                                || reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope200305())) {
                            pMsg->setFault(true);
                        }
                    }

                } else {
                    reader.raiseError(QObject::tr("Invalid SOAP Message, Body expected"));
                }
            } else {
                reader.raiseError(QObject::tr("Invalid SOAP Message, empty Envelope"));
            }
        } else {
            reader.raiseError(QObject::tr("Invalid SOAP Message, Envelope expected"));
        }
    }
    if (reader.hasError()) {
        if (reader.error() == QXmlStreamReader::NotWellFormedError) {
            qWarning() << "Handling a Not well Formed Error";
            QByteArray dataCleanedUp = handleNotWellFormedError(data, reader.characterOffset());
            if (!dataCleanedUp.isEmpty()) {
                return xmlToMessage(dataCleanedUp, pMsg, pMessageNamespace, pRequestHeaders, soapVersion);
            }
        }
        QString faultText = QString::fromLatin1("XML error: [%1:%2] %3")
                                .arg(QString::number(reader.lineNumber()), QString::number(reader.columnNumber()), reader.errorString());
        pMsg->createFaultMessage(QString::number(reader.error()), faultText, soapVersion);
        return reader.error() == QXmlStreamReader::PrematureEndOfDocumentError ? PrematureEndOfDocumentError : ParseError;
    }

    return NoError;
}
