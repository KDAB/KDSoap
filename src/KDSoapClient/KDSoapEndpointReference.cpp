/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2016-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "KDSoapEndpointReference.h"

#include <QDebug>

class KDSoapEndpointReferenceData : public QSharedData
{
public:
    KDSoapEndpointReferenceData()
    {
    }

    QString m_address;
    KDSoapValueList m_metadata;
    KDSoapValueList m_referenceParameters;
};

KDSoapEndpointReference::KDSoapEndpointReference(const QString &address)
    : d(new KDSoapEndpointReferenceData)
{
    d->m_address = address;
}

KDSoapEndpointReference::KDSoapEndpointReference(const KDSoapEndpointReference &other)
    : d(other.d)
{
}

KDSoapEndpointReference &KDSoapEndpointReference::operator=(const KDSoapEndpointReference &other)
{
    d = other.d;
    return *this;
}

KDSoapEndpointReference::~KDSoapEndpointReference()
{
}

QString KDSoapEndpointReference::address() const
{
    return d->m_address;
}

void KDSoapEndpointReference::setAddress(const QString &address)
{
    d->m_address = address;
}

KDSoapValueList KDSoapEndpointReference::metadata() const
{
    return d->m_metadata;
}

void KDSoapEndpointReference::setMetadata(const KDSoapValueList &metadata)
{
    d->m_metadata = metadata;
}

bool KDSoapEndpointReference::isEmpty() const
{
    return d->m_address.isEmpty();
}

KDSoapValueList KDSoapEndpointReference::referenceParameters() const
{
    return d->m_referenceParameters;
}

void KDSoapEndpointReference::setReferenceParameters(const KDSoapValueList &referenceParameters)
{
    d->m_referenceParameters = referenceParameters;
}
