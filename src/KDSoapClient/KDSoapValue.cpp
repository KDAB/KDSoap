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
#include "KDSoapValue.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapNamespaceManager.h"
#include "KDDateTime.h"
#include <QDateTime>
#include <QUrl>
#include <QDebug>

class KDSoapValue::Private : public QSharedData
{
public:
    Private(): m_qualified(false) {}
    Private(const QString& n, const QVariant& v, const QString& typeNameSpace, const QString& typeName)
        : m_name(n), m_value(v), m_typeNamespace(typeNameSpace), m_typeName(typeName), m_qualified(false) {}

    QString m_name;
    QString m_nameNamespace;
    QVariant m_value;
    QString m_typeNamespace;
    QString m_typeName;
    KDSoapValueList m_childValues;
    bool m_qualified;
};

uint qHash( const KDSoapValue& value ) { return qHash( value.name() ); }


KDSoapValue::KDSoapValue()
    : d(new Private)
{
}

KDSoapValue::KDSoapValue(const QString& n, const QVariant& v, const QString& typeNameSpace, const QString& typeName)
    : d(new Private(n, v, typeNameSpace, typeName))
{
}

KDSoapValue::KDSoapValue(const QString& n, const KDSoapValueList& children, const QString& typeNameSpace, const QString& typeName)
    : d(new Private(n, QVariant(), typeNameSpace, typeName))
{
    d->m_childValues = children;
}

KDSoapValue::~KDSoapValue()
{
}

KDSoapValue::KDSoapValue(const KDSoapValue& other)
    : d(other.d)
{
}

bool KDSoapValue::isNull() const
{
    return d->m_name.isEmpty() && d->m_childValues.isEmpty();
}

QString KDSoapValue::name() const
{
    return d->m_name;
}

QVariant KDSoapValue::value() const
{
    return d->m_value;
}

void KDSoapValue::setValue(const QVariant &value)
{
    d->m_value = value;
}

bool KDSoapValue::isQualified() const
{
    return d->m_qualified;
}

void KDSoapValue::setQualified(bool qualified)
{
    d->m_qualified = qualified;
}

KDSoapValueList & KDSoapValue::childValues() const
{
    // I want to fool the QSharedDataPointer mechanism here...
    return const_cast<KDSoapValueList &>(d->m_childValues);
}


bool KDSoapValue::operator ==(const KDSoapValue &other) const
{
    return d == other.d;
}

bool KDSoapValue::operator !=(const KDSoapValue &other) const
{
    return d != other.d;
}


static QString variantToTextValue(const QVariant& value, const QString& typeNs, const QString& type)
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
        const QByteArray data = value.toByteArray();
        if (typeNs == KDSoapNamespaceManager::xmlSchema1999() || typeNs == KDSoapNamespaceManager::xmlSchema2001()) {
            if (type == QLatin1String("hexBinary")) {
                const QByteArray hb = data.toHex();
                return QString::fromLatin1(hb.constData(), hb.size());
            }
        }
        // default to base64Binary, like variantToXMLType() does.
        const QByteArray b64 = value.toByteArray().toBase64();
        return QString::fromLatin1(b64.constData(), b64.size());
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
    {
        const QTime time = value.toTime();
        if (time.msec()) {
            // include milli-seconds
            return time.toString(QLatin1String("hh:mm:ss.zzz"));
        } else {
            return time.toString(Qt::ISODate);
        }
    }
    case QVariant::Date:
        return value.toDate().toString(Qt::ISODate);
    case QVariant::DateTime: // http://www.w3.org/TR/xmlschema-2/#dateTime
        return KDDateTime(value.toDateTime()).toDateString();
    case QVariant::Invalid:
        qDebug() << "ERROR: Got invalid QVariant in a KDSoapValue";
        return QString();
    default:
        if (value.canConvert<KDDateTime>()) {
            return value.value<KDDateTime>().toDateString();
        }

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
        if (value.canConvert<KDDateTime>())
            return QLatin1String("xsd:dateTime");


        qDebug() << value;

        qDebug() << QString::fromLatin1("variantToXmlType: QVariants of type %1 are not supported in "
                                        "KDSoap, see the documentation").arg(QLatin1String(value.typeName()));
        return QString();
    }
}

void KDSoapValue::writeElement(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, KDSoapValue::Use use, const QString& messageNamespace, bool forceQualified) const
{
    Q_ASSERT(!name().isEmpty());
    if ( !d->m_nameNamespace.isEmpty() && d->m_nameNamespace != messageNamespace )
        forceQualified = true;

    if (d->m_qualified || forceQualified) {
        const QString ns = d->m_nameNamespace.isEmpty() ? messageNamespace : d->m_nameNamespace;

        // TODO: if the prefix is new, we want to do namespacePrefixes.insert()
        // But this means figuring out n2/n3/n4 the same way Qt does...

        writer.writeStartElement(ns, name());
    } else {
        writer.writeStartElement(name());
    }
    writeElementContents(namespacePrefixes, writer, use, messageNamespace);
    writer.writeEndElement();
}

void KDSoapValue::writeElementContents(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, KDSoapValue::Use use, const QString& messageNamespace) const
{
    const QVariant value = this->value();
    const KDSoapValueList list = this->childValues();

    if (value.isNull() && list.isEmpty())
        writer.writeAttribute(KDSoapNamespaceManager::xmlSchemaInstance2001(), QLatin1String("nil"), QLatin1String("true"));

    if (use == EncodedUse) {
        // use=encoded means writing out xsi:type attributes. http://www.eherenow.com/soapfight.htm taught me that.
        QString type;
        if (!this->type().isEmpty())
            type = namespacePrefixes.resolve(this->typeNs(), this->type());
        if (type.isEmpty() && !value.isNull())
            type = variantToXMLType(value); // fallback
        if (!type.isEmpty()) {
            writer.writeAttribute(KDSoapNamespaceManager::xmlSchemaInstance2001(), QLatin1String("type"), type);
        }

        const bool isArray = !list.arrayType().isEmpty();
        if (isArray) {
            writer.writeAttribute(KDSoapNamespaceManager::soapEncoding(), QLatin1String("arrayType"), namespacePrefixes.resolve(list.arrayTypeNs(), list.arrayType()) + QLatin1Char('[') + QString::number(list.count()) + QLatin1Char(']'));
        }
    }
    writeChildren(namespacePrefixes, writer, use, messageNamespace, false);

    if (!value.isNull())
        writer.writeCharacters(variantToTextValue(value, this->typeNs(), this->type()));
}

void KDSoapValue::writeChildren(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, KDSoapValue::Use use, const QString& messageNamespace, bool forceQualified) const
{
    const KDSoapValueList& args = childValues();
    Q_FOREACH(const KDSoapValue& attr, args.attributes()) {
        //Q_ASSERT(!attr.value().isNull());
        QString ns;
        if ( !d->m_nameNamespace.isEmpty() && d->m_nameNamespace != messageNamespace )
            forceQualified = true;
        if (d->m_qualified || forceQualified)
            ns = d->m_nameNamespace.isEmpty() ? messageNamespace : d->m_nameNamespace;
        writer.writeAttribute(ns, attr.name(), variantToTextValue(attr.value(), attr.typeNs(), attr.type()));
    }
    KDSoapValueListIterator it(args);
    while (it.hasNext()) {
        const KDSoapValue& element = it.next();
        element.writeElement(namespacePrefixes, writer, use, messageNamespace, forceQualified);
    }
}

////

QDebug operator <<(QDebug dbg, const KDSoapValue &value)
{
    dbg.space() << value.name() << value.value();
    if (!value.childValues().isEmpty()) {
        dbg << "<children>";
        KDSoapValueListIterator it(value.childValues());
        while (it.hasNext()) {
            const KDSoapValue& child = it.next();
            dbg << child;
        }
        dbg << "</children>";
    }
    if (!value.childValues().attributes().isEmpty()) {
        dbg << "<attributes>";
        QListIterator<KDSoapValue> it(value.childValues().attributes());
        while (it.hasNext()) {
            const KDSoapValue& child = it.next();
            dbg << child;
        }
        dbg << "</attributes>";
    }
    return dbg;
}

void KDSoapValue::setType(const QString& nameSpace, const QString &type)
{
    d->m_typeNamespace = nameSpace;
    d->m_typeName = type;
}

QString KDSoapValue::typeNs() const
{
    return d->m_typeNamespace;
}

QString KDSoapValue::type() const
{
    return d->m_typeName;
}

KDSoapValue KDSoapValueList::child(const QString &name) const
{
    const_iterator it = begin();
    const const_iterator e = end();
    for ( ; it != e; ++it) {
        const KDSoapValue& val = *it;
        if (val.name() == name)
            return val;
    }
    return KDSoapValue();
}

void KDSoapValueList::setArrayType(const QString& nameSpace, const QString &type)
{
    m_arrayType = qMakePair(nameSpace, type);
}

QString KDSoapValueList::arrayTypeNs() const
{
    return m_arrayType.first;
}

QString KDSoapValueList::arrayType() const
{
    return m_arrayType.second;
}

void KDSoapValueList::addArgument(const QString& argumentName, const QVariant& argumentValue, const QString& typeNameSpace, const QString& typeName)
{
    append(KDSoapValue(argumentName, argumentValue, typeNameSpace, typeName));
}

QString KDSoapValue::namespaceUri() const
{
    return d->m_nameNamespace;
}

void KDSoapValue::setNamespaceUri(const QString &ns)
{
    d->m_nameNamespace = ns;
}

QByteArray KDSoapValue::toXml(KDSoapValue::Use use, const QString& messageNamespace) const
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.writeStartDocument();

    KDSoapNamespacePrefixes namespacePrefixes;
    namespacePrefixes.writeStandardNamespaces(writer);

    writeElement(namespacePrefixes, writer, use, messageNamespace, false);
    writer.writeEndDocument();

    return data;
}
