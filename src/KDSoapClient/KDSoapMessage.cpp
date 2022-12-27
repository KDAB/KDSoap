/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#include "KDSoapMessage.h"
#include "KDDateTime.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapNamespacePrefixes_p.h"
#include <QDebug>
#include <QVariant>
#include <QXmlStreamReader>

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapMessageData()
        : use(KDSoapMessage::LiteralUse)
        , isFault(false)
        , hasMessageAddressingProperties(false)
    {
    }

    KDSoapMessage::Use use;
    bool isFault;
    bool hasMessageAddressingProperties;
    KDSoapMessageAddressingProperties messageAddressingProperties;
};

KDSoapMessage::KDSoapMessage()
    : d(new KDSoapMessageData)
{
}

KDSoapMessage::KDSoapMessage(const KDSoapMessage &other)
    : KDSoapValue(other)
    , d(other.d)
{
}

KDSoapMessage &KDSoapMessage::operator=(const KDSoapMessage &other)
{
    KDSoapValue::operator=(other);
    d = other.d;
    return *this;
}

KDSoapMessage &KDSoapMessage::operator=(const KDSoapValue &other)
{
    KDSoapValue::operator=(other);
    return *this;
}

bool KDSoapMessage::operator==(const KDSoapMessage &other) const
{
    return KDSoapValue::operator==(other) && d->use == other.d->use && d->isFault == other.d->isFault;
}

bool KDSoapMessage::operator!=(const KDSoapMessage &other) const
{
    return !(*this == other);
}

KDSoapMessage::~KDSoapMessage()
{
}

void KDSoapMessage::addArgument(const QString &argumentName, const QVariant &argumentValue, const QString &typeNameSpace, const QString &typeName)
{
    KDSoapValue soapValue(argumentName, argumentValue, typeNameSpace, typeName);
    if (isQualified()) {
        soapValue.setQualified(true);
    }
    childValues().append(soapValue);
}

void KDSoapMessage::addArgument(const QString &argumentName, const KDSoapValueList &argumentValueList, const QString &typeNameSpace,
                                const QString &typeName)
{
    KDSoapValue soapValue(argumentName, argumentValueList, typeNameSpace, typeName);
    if (isQualified()) {
        soapValue.setQualified(true);
    }
    childValues().append(soapValue);
}

// I'm leaving the arguments() method even though it's the same as childValues,
// because it's the documented public API, needed even in the most simple case,
// while childValues is the "somewhat internal" KDSoapValue stuff.

KDSoapValueList &KDSoapMessage::arguments()
{
    return childValues();
}

const KDSoapValueList &KDSoapMessage::arguments() const
{
    return childValues();
}

QDebug operator<<(QDebug dbg, const KDSoapMessage &msg)
{
    return dbg << KDSoapValue(msg);
}

bool KDSoapMessage::isFault() const
{
    return d->isFault;
}

QString KDSoapMessage::faultAsString() const
{
    if (namespaceUri() == QLatin1String("http://www.w3.org/2003/05/soap-envelope")) {
        QString faultCodeStr;
        KDSoapValue faultCode = childValues().child(QLatin1String("Code"));
        while (!faultCode.isNull()) {
            if (!faultCodeStr.isEmpty()) {
                faultCodeStr += QLatin1String(" ");
            }
            faultCodeStr += faultCode.childValues().child(QLatin1String("Value")).value().toString();
            faultCode = faultCode.childValues().child(QLatin1String("Subcode"));
        }
        const QString faultText =
            childValues().child(QLatin1String("Reason")).childValues().child(QLatin1String("Text")).value().toString();
        return QObject::tr("Fault %1: %2")
            .arg(faultCodeStr, faultText);
    } else {
        // This better be on a single line, since it's used by server-side logging too
        const QString actor = childValues().child(QLatin1String("faultactor")).value().toString();
        QString ret = QObject::tr("Fault code %1: %2%3")
                          .arg(childValues().child(QLatin1String("faultcode")).value().toString(),
                               childValues().child(QLatin1String("faultstring")).value().toString(),
                               actor.isEmpty() ? QString() : QString::fromLatin1(" (%1)").arg(actor));
        const QString detail = childValues().child(QLatin1String("detail")).value().toString();
        if (!detail.isEmpty()) {
            if (!ret.endsWith(QLatin1Char('.'))) {
                ret += QLatin1Char('.');
            }
            ret += QLatin1String(" Error detail: ") + detail;
        }
        return ret;
    }
}

void KDSoapMessage::setFault(bool fault)
{
    d->isFault = fault;
}

void KDSoapMessage::createFaultMessage(const QString &faultCode, const QString &faultText, KDSoap::SoapVersion soapVersion)
{
    *this = KDSoapMessage();
    setName(QString::fromLatin1("Fault"));
    d->isFault = true;
    if (soapVersion == KDSoap::SOAP1_2) {
        setNamespaceUri(KDSoapNamespaceManager::soapEnvelope200305());
        KDSoapValueList codeValueList;
        codeValueList.addArgument(QString::fromLatin1("Value"), faultCode);
        addArgument(QString::fromLatin1("Code"), codeValueList);
        KDSoapValueList reasonValueList;
        reasonValueList.addArgument(QString::fromLatin1("Text"), faultText);
        addArgument(QString::fromLatin1("Reason"), reasonValueList);
    } else {
        setNamespaceUri(KDSoapNamespaceManager::soapEnvelope());
        addArgument(QString::fromLatin1("faultcode"), faultCode);
        addArgument(QString::fromLatin1("faultstring"), faultText);
    }
}

KDSoapMessageAddressingProperties KDSoapMessage::messageAddressingProperties() const
{
    return d->messageAddressingProperties;
}

void KDSoapMessage::setMessageAddressingProperties(const KDSoapMessageAddressingProperties &map)
{
    d->messageAddressingProperties = map;
    d->hasMessageAddressingProperties = true;
}

bool KDSoapMessage::hasMessageAddressingProperties() const
{
    return d->hasMessageAddressingProperties;
}

KDSoapMessage::Use KDSoapMessage::use() const
{
    return d->use;
}

void KDSoapMessage::setUse(Use use)
{
    d->use = use;
}

KDSoapMessage KDSoapHeaders::header(const QString &name) const
{
    for (const KDSoapMessage &header : qAsConst(*this)) {
        if (header.name() == name) {
            return header;
        }
    }
    return KDSoapMessage();
}

KDSoapMessage KDSoapHeaders::header(const QString &name, const QString &namespaceUri) const
{
    for (const KDSoapMessage &header : qAsConst(*this)) {
        // qDebug() << "header(" << name << "," << namespaceUri << "): Looking at" << header.name() << "," << header.namespaceUri();
        if (header.name() == name && (namespaceUri.isEmpty() || header.namespaceUri() == namespaceUri)) {
            return header;
        }
    }
    return KDSoapMessage();
}

bool KDSoapMessage::isNull() const
{
    return childValues().isEmpty() && childValues().attributes().isEmpty() && value().isNull();
}
