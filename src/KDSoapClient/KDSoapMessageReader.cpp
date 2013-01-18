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

#include "KDSoapMessageReader_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDDateTime.h"

#include <QXmlStreamReader>

// Wrapper for compatibility with Qt < 4.6.
static bool readNextStartElement(QXmlStreamReader& reader)
{
#if QT_VERSION >= 0x040600
    return reader.readNextStartElement();
#else
    while (reader.readNext() != QXmlStreamReader::Invalid) {
        if (reader.isEndElement())
            return false;
        else if (reader.isStartElement())
            return true;
    }
    return false;
#endif
}

static QStringRef namespaceForPrefix(const QXmlStreamNamespaceDeclarations& decls, const QString& prefix)
{
    for (int i = 0; i < decls.count(); ++i) {
        const QXmlStreamNamespaceDeclaration& decl = decls.at(i);
        if (decl.prefix() == prefix)
            return decl.namespaceUri();
    }
    return QStringRef();
}

static int xmlTypeToMetaType(const QString& xmlType)
{
    // Reverse operation from variantToXmlType in KDSoapClientInterface, keep in sync
    static const struct {
        const char* xml; // xsd: prefix assumed
        const int metaTypeId;
    } s_types[] = {
        { "string", QVariant::String }, // or QUrl
        { "base64Binary", QVariant::ByteArray },
        { "int", QVariant::Int }, // or long, or uint, or longlong
        { "unsignedInt", QVariant::ULongLong },
        { "boolean", QVariant::Bool },
        { "float", QMetaType::Float },
        { "double", QVariant::Double },
        { "time", QVariant::Time },
        { "date", QVariant::Date }
    };
    // Speed: could be sorted and then we could use qBinaryFind
    static const int s_numTypes = sizeof(s_types) / sizeof(*s_types);
    for (int i = 0; i < s_numTypes; ++i) {
        if (xmlType == QLatin1String(s_types[i].xml)) {
            return s_types[i].metaTypeId;
        }
    }
    if (xmlType == QLatin1String("dateTime"))
        return qMetaTypeId<KDDateTime>();
    // This will happen with any custom type, don't bother the user
    //qDebug() << QString::fromLatin1("xmlTypeToMetaType: XML type %1 is not supported in "
    //                                "KDSoap, see the documentation").arg(xmlType);
    return -1;
}

static KDSoapValue parseElement(QXmlStreamReader& reader, const QXmlStreamNamespaceDeclarations& envNsDecls)
{
    const QString name = reader.name().toString();
    KDSoapValue val(name, QVariant());
    val.setNamespaceUri(reader.namespaceUri().toString());
    //qDebug() << "parsing" << name;
    QVariant::Type metaTypeId = QVariant::Invalid;

    const QXmlStreamAttributes attributes = reader.attributes();
    Q_FOREACH(const QXmlStreamAttribute& attribute, attributes) {
        const QStringRef name = attribute.name();
        const QStringRef ns = attribute.namespaceUri();
        const QStringRef attrValue = attribute.value();
        // Parse xsi:type and soap-enc:arrayType
        // and ignore anything else from the xsi or soap-enc namespaces until someone needs it...
        if (ns == KDSoapNamespaceManager::xmlSchemaInstance1999() ||
            ns == KDSoapNamespaceManager::xmlSchemaInstance2001()) {
            if (name == QLatin1String("type")) {
                // The type can be like xsd:float, resolve that
                const QString type = attrValue.toString();
                const int pos = type.indexOf(QLatin1Char(':'));
                const QString dataType = type.mid(pos+1);
                val.setType(namespaceForPrefix(envNsDecls, type.left(pos)).toString(), dataType);
                metaTypeId = static_cast<QVariant::Type>(xmlTypeToMetaType(dataType));
            }
            continue;
        } else if (ns == KDSoapNamespaceManager::soapEncoding() || ns == KDSoapNamespaceManager::soapEncoding200305() ||
                   ns == KDSoapNamespaceManager::soapEnvelope() || ns == KDSoapNamespaceManager::soapEnvelope200305()) {
            continue;
        }
        //qDebug() << "Got attribute:" << name << ns << "=" << attrValue;
        val.childValues().attributes().append(KDSoapValue(name.toString(), attrValue.toString()));
    }
    QString text;
    while (reader.readNext() != QXmlStreamReader::Invalid) {
        if (reader.isEndElement())
            break;
        if (reader.isCharacters()) {
            text = reader.text().toString();
            //qDebug() << "text=" << text;
        } else if (reader.isStartElement()) {
            const KDSoapValue subVal = parseElement(reader, envNsDecls); // recurse
            val.childValues().append(subVal);
        }
    }

    if (!text.isEmpty()) {
        QVariant variant(text);
        //qDebug() << text << variant << metaTypeId;
        // With use=encoded, we have type info, we can convert the variant here
        // Otherwise, for servers, we do it later, once we know the method's parameter types.
        if (metaTypeId != QVariant::Invalid) {
            QVariant copy = variant;
            if (!variant.convert(metaTypeId))
                variant = copy;
        }
        val.setValue(variant);
    }
    return val;
}

KDSoapMessageReader::KDSoapMessageReader()
{
}

KDSoapMessageReader::XmlError KDSoapMessageReader::xmlToMessage(const QByteArray& data, KDSoapMessage* pMsg, QString* pMessageNamespace, KDSoapHeaders* pRequestHeaders) const
{
    Q_ASSERT(pMsg);
    QXmlStreamReader reader(data);
    if (readNextStartElement(reader)) {
        if (reader.name() == QLatin1String("Envelope") && (reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope() ||
                                            reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope200305())) {
            const QXmlStreamNamespaceDeclarations envNsDecls = reader.namespaceDeclarations();
            if (readNextStartElement(reader)) {
                if (reader.name() == QLatin1String("Header") && (reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope() ||
                                                  reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope200305())) {
                    while (readNextStartElement(reader)) {
                        KDSoapMessage header;
                        static_cast<KDSoapValue &>(header) = parseElement(reader, envNsDecls);
                        pRequestHeaders->append(header);
                    }
                    readNextStartElement(reader); // read <Body>
                }
                if (reader.name() == QLatin1String("Body") && (reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope() ||
                                                reader.namespaceUri() == KDSoapNamespaceManager::soapEnvelope200305())) {
                    if (readNextStartElement(reader)) {
                        *pMsg = parseElement(reader, envNsDecls);
                        if (pMessageNamespace)
                            *pMessageNamespace = pMsg->namespaceUri();
                        if (pMsg->name() == QLatin1String("Fault"))
                            pMsg->setFault(true);
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
        pMsg->setFault(true);
        pMsg->addArgument(QString::fromLatin1("faultcode"), QString::number(reader.error()));
        pMsg->addArgument(QString::fromLatin1("faultstring"),
                          QString::fromLatin1("XML error: [%1:%2] %3").arg(QString::number(reader.lineNumber()),
                                                                           QString::number(reader.columnNumber()), 
                                                                           reader.errorString()));
        return reader.error() == QXmlStreamReader::PrematureEndOfDocumentError ? PrematureEndOfDocumentError : ParseError;
    }

    return NoError;
}
