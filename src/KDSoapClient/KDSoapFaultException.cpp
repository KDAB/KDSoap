/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

#include "KDSoapFaultException.h"

class KDSoapFaultException::Private: public QSharedData
{
public:
    Private();
public:
    QString m_faultCode;
    QString m_faultString;
    QString m_faultActor;
    KDSoapValue m_detailValue;
};

KDSoapFaultException::Private::Private()
{
}

KDSoapFaultException::KDSoapFaultException():
    d(new Private())
{
}

KDSoapFaultException::KDSoapFaultException(const KDSoapFaultException &cpy)
    :d(cpy.d)
{

}

KDSoapFaultException::KDSoapFaultException(const QString &faultCode, const QString &faultString, const QString &faultActor):
     d(new Private())
{
    d->m_faultCode = faultCode;
    d->m_faultString = faultString;
    d->m_faultActor = faultActor;
}

KDSoapFaultException &KDSoapFaultException::operator=(const KDSoapFaultException &other)
{
    if (this == &other)
        return *this;

    d = other.d;
    return *this;
}

KDSoapFaultException::~KDSoapFaultException()
{
}

void KDSoapFaultException::deserialize( const KDSoapValue& mainValue )
{
    Q_ASSERT(mainValue.name() == QLatin1String("Fault"));
    const KDSoapValueList& args = mainValue.childValues();
    for (int argNr = 0; argNr < args.count(); ++argNr) {
        const KDSoapValue& val = args.at(argNr);
        const QString name = val.name();
        if (name == QLatin1String("faultcode")) {
            d->m_faultCode = val.value().value<QString>();
        }
        else if (name == QLatin1String("faultstring")) {
            d->m_faultString = val.value().value<QString>();
        }
        else if (name == QLatin1String("faultactor")) {
            d->m_faultActor = val.value().value<QString>();
        }
    }
}

QString KDSoapFaultException::faultCode() const
{
    return d->m_faultCode;
}

const KDSoapValue &KDSoapFaultException::faultDetails(const KDSoapValue &faultValue)
{
    static KDSoapValue emptyValue;

    // Find and return the <detail> element under faultElement
    const KDSoapValueList& args = faultValue.childValues();
    for (int argNr = 0; argNr < args.count(); ++argNr) {
        const KDSoapValue& val = args.at(argNr);
        const QString name = val.name();
        if (name == QLatin1String("detail")) {
            return val;
        }
    }
    return emptyValue;
}

void KDSoapFaultException::setFaultCode(const QString &faultCode)
{
    d->m_faultCode = faultCode;
}
QString KDSoapFaultException::faultString() const
{
    return d->m_faultString;
}

void KDSoapFaultException::setFaultString(const QString &faultString)
{
    d->m_faultString = faultString;
}

QString KDSoapFaultException::faultActor() const
{
    return d->m_faultActor;
}

void KDSoapFaultException::setFaultActor(const QString &faultActor)
{
    d->m_faultActor = faultActor;
}

KDSoapValue KDSoapFaultException::detailValue() const
{
    return d->m_detailValue;
}

void KDSoapFaultException::setDetailValue(const KDSoapValue &detailValue)
{
    d->m_detailValue = detailValue;
}
