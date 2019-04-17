/****************************************************************************
** Copyright (C) 2010-2019 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#include "KDSoapMessage.h"
#include "KDSoapNamespaceManager.h"
#include "KDSoapNamespacePrefixes_p.h"
#include "KDDateTime.h"
#include <QDebug>
#include <QXmlStreamReader>
#include <QVariant>

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapMessageData()
        : use(KDSoapMessage::LiteralUse), isFault(false), hasMessageAddressingProperties(false)
    {}

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
    : KDSoapValue(other), d(other.d)
{
}

KDSoapMessage &KDSoapMessage::operator =(const KDSoapMessage &other)
{
    KDSoapValue::operator=(other);
    d = other.d;
    return *this;
}

KDSoapMessage &KDSoapMessage::operator =(const KDSoapValue &other)
{
    KDSoapValue::operator=(other);
    return *this;
}

bool KDSoapMessage::operator==(const KDSoapMessage &other) const
{
    return KDSoapValue::operator ==(other)
           && d->use == other.d->use
           && d->isFault == other.d->isFault;
}

bool KDSoapMessage::operator !=(const KDSoapMessage &other) const
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

void KDSoapMessage::addArgument(const QString &argumentName, const KDSoapValueList &argumentValueList, const QString &typeNameSpace, const QString &typeName)
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

QDebug operator <<(QDebug dbg, const KDSoapMessage &msg)
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
            if (!faultCodeStr.isEmpty())
                faultCodeStr += QLatin1String(" ");
            faultCodeStr += faultCode.childValues().child(QLatin1String("Value")).value().toString();
            faultCode = faultCode.childValues().child(QLatin1String("Subcode"));
        }
        return QObject::tr("Fault %1: %2")
                .arg(faultCodeStr)
                .arg(childValues().child(QLatin1String("Reason")).childValues().child(QLatin1String("Text")).value().toString());
    } else {
        // This better be on a single line, since it's used by server-side logging too
        const QString actor = childValues().child(QLatin1String("faultactor")).value().toString();
        QString ret = QObject::tr("Fault code %1: %2%3")
               .arg(childValues().child(QLatin1String("faultcode")).value().toString(),
                    childValues().child(QLatin1String("faultstring")).value().toString(),
                    actor.isEmpty() ? QString() : QString::fromLatin1(" (%1)").arg(actor));
        const QString detail = childValues().child(QLatin1String("detail")).value().toString();
        if (!detail.isEmpty()) {
            if (!ret.endsWith(QLatin1Char('.')))
                ret += QLatin1Char('.');
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
    const_iterator it = begin();
    const const_iterator e = end();
    for (; it != e; ++it) {
        if ((*it).name() == name) {
            return *it;
        }
    }
    return KDSoapMessage();
}

KDSoapMessage KDSoapHeaders::header(const QString &name, const QString &namespaceUri) const
{
    const_iterator it = begin();
    const const_iterator e = end();
    for (; it != e; ++it) {
        //qDebug() << "header(" << name << "," << namespaceUri << "): Looking at" << (*it).name() << "," << (*it).namespaceUri();
        if ((*it).name() == name && (namespaceUri.isEmpty() || (*it).namespaceUri() == namespaceUri)) {
            return *it;
        }
    }
    return KDSoapMessage();
}

bool KDSoapMessage::isNull() const
{
    return childValues().isEmpty() && childValues().attributes().isEmpty() && value().isNull();
}
