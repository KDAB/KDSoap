#include "KDSoapMessageWriter_p.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapValue.h"
#include <QVariant>
#include <QDateTime>
#include <QUrl>
#include <QDebug>

KDSoapMessageWriter::KDSoapMessageWriter()
{
}

void KDSoapMessageWriter::setMessageNamespace(const QString &ns)
{
    m_messageNamespace = ns;
}

static QString variantToTextValue(const QVariant& value)
{
    switch (value.userType())
    {
    case QVariant::Char:
        // fall-through
    case QVariant::String:
        return value.toString();
    case QVariant::Url:
        // xmlpatterns/data/qatomicvalue.cpp says to do this:
        return value.toUrl().toString();
    case QVariant::ByteArray:
        {
            const QByteArray b64 = value.toByteArray().toBase64();
            return QString::fromLatin1(b64.data(),b64.size());
        }
    case QVariant::Int:
        // fall-through
    case QVariant::LongLong:
        // fall-through
    case QVariant::UInt:
        return QString::number(value.toLongLong());
    case QVariant::ULongLong:
        return QString::number(value.toULongLong());
    case QVariant::Bool:
    case QMetaType::Float:
    case QVariant::Double:
        return value.toString();
    case QVariant::Time:
        return value.toDateTime().toString(Qt::ISODate);
    case QVariant::Date:
        return QDateTime(value.toDate(), QTime(), Qt::UTC).toString(Qt::ISODate);
    case QVariant::DateTime:
        return value.toDateTime().toString(Qt::ISODate);
    case QVariant::Invalid:
        qDebug() << "ERROR: Got invalid QVariant in a KDSoapValue";
        return QString();
    default:
        if (value.userType() == qMetaTypeId<float>())
            return QString::number(value.value<float>());

        qDebug() << QString::fromLatin1("QVariants of type %1 are not supported in "
                                        "KDSoap, see the documentation").arg(QLatin1String(value.typeName()));
        return value.toString();
    }
}

// See also xmlTypeToVariant in serverlib
static QString variantToXMLType(const QVariant& value)
{
    switch (value.userType())
    {
    case QVariant::Char:
        // fall-through
    case QVariant::String:
        // fall-through
    case QVariant::Url:
        return QLatin1String("xsd:string");
    case QVariant::ByteArray:
        return QLatin1String("xsd:base64Binary");
    case QVariant::Int:
        // fall-through
    case QVariant::LongLong:
        // fall-through
    case QVariant::UInt:
        return QLatin1String("xsd:int");
    case QVariant::ULongLong:
        return QLatin1String("xsd:unsignedInt");
    case QVariant::Bool:
        return QLatin1String("xsd:boolean");
    case QMetaType::Float:
        return QLatin1String("xsd:float");
    case QVariant::Double:
        return QLatin1String("xsd:double");
    case QVariant::Time:
        return QLatin1String("xsd:time"); // correct? xmlpatterns fallsback to datetime because of missing timezone
    case QVariant::Date:
        return QLatin1String("xsd:date");
    case QVariant::DateTime:
        return QLatin1String("xsd:dateTime");
    default:
        if (value.userType() == qMetaTypeId<float>())
            return QLatin1String("xsd:float");

        qDebug() << value;

        qDebug() << QString::fromLatin1("variantToXmlType: QVariants of type %1 are not supported in "
                                        "KDSoap, see the documentation").arg(QLatin1String(value.typeName()));
        return QString();
    }
}

QByteArray KDSoapMessageWriter::messageToXml(const KDSoapMessage& message, const QString& method,
                                             const KDSoapHeaders& headers, const QMap<QString, KDSoapMessage>& persistentHeaders) const
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.writeStartDocument();

    KDSoapNamespacePrefixes namespacePrefixes;

    const QString soapNS = KDSoapNamespaceManager::soapEnvelope();
    namespacePrefixes.writeNamespace(writer, soapNS, QLatin1String("soap"));
    namespacePrefixes.writeNamespace(writer, KDSoapNamespaceManager::soapEncoding(), QLatin1String("soap-enc"));
    namespacePrefixes.writeNamespace(writer, KDSoapNamespaceManager::xmlSchema1999(), QLatin1String("xsd"));
    namespacePrefixes.writeNamespace(writer, KDSoapNamespaceManager::xmlSchemaInstance1999(), QLatin1String("xsi"));

    // Also insert known variants
    namespacePrefixes.insert(KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("xsd"));
    namespacePrefixes.insert(KDSoapNamespaceManager::xmlSchemaInstance2001(), QString::fromLatin1("xsi"));

    writer.writeStartElement(soapNS, QLatin1String("Envelope"));
    writer.writeAttribute(soapNS, QLatin1String("encodingStyle"), KDSoapNamespaceManager::soapEncoding());

    if (!headers.isEmpty() || !persistentHeaders.isEmpty()) {
        // This writeNamespace line adds the xmlns:n1 to <Envelope>, which looks ugly and unusual (and breaks all unittests)
        // However it's the best solution in case of headers, otherwise we get n1 in the header and n2 in the body,
        // and xsi:type attributes that refer to n1, which isn't defined in the body...
        namespacePrefixes.writeNamespace(writer, m_messageNamespace, QLatin1String("n1") /*make configurable?*/);
        writer.writeStartElement(soapNS, QLatin1String("Header"));
        Q_FOREACH(const KDSoapMessage& header, persistentHeaders) {
            writeChildren(namespacePrefixes, writer, header.childValues(), header.use());
        }
        Q_FOREACH(const KDSoapMessage& header, headers) {
            writeChildren(namespacePrefixes, writer, header.childValues(), header.use());
        }
        writer.writeEndElement(); // Header
    } else {
        // So in the standard case (no headers) we just rely on Qt calling it n1 and insert it into the map.
        // Calling this after the writeStartElement(method) below leads to a double-definition of n1.
        namespacePrefixes.insert(m_messageNamespace, QString::fromLatin1("n1"));
    }

    writer.writeStartElement(soapNS, QLatin1String("Body"));

    writer.writeStartElement(m_messageNamespace, method);
    writeElementContents(namespacePrefixes, writer, message, message.use());
    writer.writeEndElement(); // <method>

    writer.writeEndElement(); // Body
    writer.writeEndElement(); // Envelope
    writer.writeEndDocument();

    if (qgetenv("KDSOAP_DEBUG").toInt()) {
        qDebug() << data;
    }
    return data;
}

void KDSoapMessageWriter::writeElementContents(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValue& element, KDSoapMessage::Use use) const
{
    const QVariant value = element.value();
    const KDSoapValueList list = element.childValues();
    if (use == KDSoapMessage::EncodedUse) {
        // use=encoded means writing out xsi:type attributes. http://www.eherenow.com/soapfight.htm taught me that.
        QString type;
        if (!element.type().isEmpty())
            type = namespacePrefixes.resolve(element.typeNs(), element.type());
        if (type.isEmpty() && !value.isNull())
            type = variantToXMLType(value); // fallback
        if (!type.isEmpty()) {
            writer.writeAttribute(KDSoapNamespaceManager::xmlSchemaInstance1999(), QLatin1String("type"), type);
        }

        const bool isArray = !list.arrayType().isEmpty();
        if (isArray) {
            writer.writeAttribute(KDSoapNamespaceManager::soapEncoding(), QLatin1String("arrayType"), namespacePrefixes.resolve(list.arrayTypeNs(), list.arrayType()) + QLatin1Char('[') + QString::number(list.count()) + QLatin1Char(']'));
        }
    }
    writeChildren(namespacePrefixes, writer, list, use);

    if (!value.isNull())
        writer.writeCharacters(variantToTextValue(value));
}

void KDSoapMessageWriter::writeChildren(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValueList& args, KDSoapMessage::Use use) const
{
    writeAttributes(writer, args.attributes());
    KDSoapValueListIterator it(args);
    while (it.hasNext()) {
        const KDSoapValue& element = it.next();
        writer.writeStartElement(m_messageNamespace, element.name());
        writeElementContents(namespacePrefixes, writer, element, use);
        writer.writeEndElement();
    }
}

void KDSoapMessageWriter::writeAttributes(QXmlStreamWriter& writer, const QList<KDSoapValue>& attributes) const
{
    Q_FOREACH(const KDSoapValue& attr, attributes) {
        Q_ASSERT(!attr.value().isNull());
        writer.writeAttribute(m_messageNamespace, attr.name(), variantToTextValue(attr.value()));
    }
}
